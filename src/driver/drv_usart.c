/**
 * @file drv_usart.c
 * @author Alexey Nikolaev (alexeynikzzz@gmail.com)
 * @brief USART driver implementation for stm32f103xx series
 *        USART1 (TX/PA9, RX/PA10) for debug print
 *        USART2 (TX/PA2, RX/PA3) for one wire
 * @version 0.1
 * @date 2020-04-26
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#include "drv_usart.h"
#include "stm32f103xb.h"
#include "drv_clocks.h"

#include <math.h>
#include <string.h>

volatile static struct xUsartContext_t
{
    struct 
    {
        xDrvUsartPortParams_t port;
        BOOL is_hw_inited;
    } usart[DU_USART_NUM];
    
    
} _cxt;

static result_t _calculate_usartdiv_for_baudrate(xDrvUsartPortParams_t * port_params, uint32_t * usart_div);
static USART_TypeDef * _get_usart_registers_struct(eDrvUsartNum_t usart_no);

INLINE result_t _init_hw_usart1(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;

    //TX pin
    GPIOA->CRH &= ~(GPIO_CRH_MODE9 | GPIO_CRH_CNF9); //Clear PA9 configuration
    GPIOA->CRH |= GPIO_CRH_MODE9_1 | GPIO_CRH_CNF9_1; //Set PA9 to output 2MHz, alternate push-pull

    //RX pin
    GPIOA->CRH &= ~(GPIO_CRH_MODE10 | GPIO_CRH_CNF10); //Clear PA10 configuration
    GPIOA->CRH |= GPIO_CRH_CNF10_0; //Set PA10 to floating input
    //GPIOA->BSRR |= GPIO_ODR_ODR10;

    AFIO->MAPR &= ~AFIO_MAPR_USART1_REMAP;
    return RESULT_OK;
}

INLINE result_t _init_hw_usart2(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;

    //TX pin
    GPIOA->CRL &= ~(GPIO_CRL_MODE2 | GPIO_CRL_CNF2); //Clear PA2 configuration
    GPIOA->CRL |= GPIO_CRL_MODE2_1 | GPIO_CRL_CNF2_1 | GPIO_CRL_CNF2_0; //Set PA2 to output 2MHz, alternate push-pull

    // GPIOA->CRL &= ~(GPIO_CRL_MODE2 | GPIO_CRL_CNF2); //Clear PA9 configuration
    // GPIOA->CRL |= GPIO_CRL_MODE2_1; //Set PA9 to output 2MHz, alternate push-pull

    //RX pin
    GPIOA->CRL &= ~(GPIO_CRL_MODE3 | GPIO_CRL_CNF3); //Clear PA3 configuration
    GPIOA->CRL |= GPIO_CRL_CNF3_0; //Set PA3 to floating input
    //GPIOA->BSRR |= GPIO_ODR_ODR10;

    AFIO->MAPR &= ~AFIO_MAPR_USART2_REMAP;
    return RESULT_OK;
}

INLINE result_t _init_hw_usart3(void)
{
    return RESULT_OK;
}

static result_t _init_hw(xDrvUsartPortParams_t * port_params)
{
    eDrvUsartNum_t usart_no = port_params->usart_num;
    result_t res = RESULT_OK;

    switch(usart_no)
    {
        case DU_USART1:
            res = _init_hw_usart1();
            break;
        case DU_USART2:
            res = _init_hw_usart2();
            break;
        case DU_USART3:
            res = _init_hw_usart3();
            break;

    }

    return res;
}

/**
 * @brief USART port initialization
 * 
 * @param port_params The parameters to use for port initialization
 * @return result_t Operation result
 */
result_t drv_usart_init_port(xDrvUsartPortParams_t * port_params)
{
    result_t res = RESULT_OK;
    USART_TypeDef * usart = _get_usart_registers_struct(port_params->usart_num);
    
    //Need to disable USART to reset pending flags in status register
    usart->CR1 &= ~USART_CR1_UE;

    res = _init_hw(port_params);
    ASSERT("USART init hw failed", res == RESULT_OK);
    _cxt.usart[port_params->usart_num].is_hw_inited = TRUE;

    usart->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;   //Enable USART with Rx and Tx lines
    uint32_t usart_div;
    _calculate_usartdiv_for_baudrate(port_params, &usart_div);
    usart->BRR = usart_div;    

    usart->CR1 |= USART_CR1_RXNEIE;
    usart->DR;
    //TODO: make parity, word length, stop bits settings

    memcpy((void *) &_cxt.usart[port_params->usart_num].port, port_params, sizeof(xDrvUsartPortParams_t));
    return res;
}

/**
 * @brief Get the value of USART clk
 * 
 * @param usart_no The number of USART port
 * @return Clock value 
 */
uint32_t drv_usart_get_clock(eDrvUsartNum_t usart_no)
{
    uint32_t usart_clock = 0;
    if (usart_no == DU_USART1)
    {
        usart_clock = drv_clocks_get_pclk2();
    }
    else
    {
        usart_clock = drv_clocks_get_pclk1();
    }

    return usart_clock;
}

/**
 * @brief Function to calculate the USART baudrate
 * 
 * @param port_params Parameters for the port
 * @param usart_div The pointer to the variable containing the divider value
 * @return result_t Operation result
 */
static result_t _calculate_usartdiv_for_baudrate(xDrvUsartPortParams_t * port_params, uint32_t * usart_div)
{
    eDrvUsartNum_t usart_no = port_params->usart_num;
    uint32_t baudrate = port_params->baudrate;
    uint32_t frequency = drv_usart_get_clock(usart_no);

    *usart_div = (uint32_t) roundf((float) frequency / (float) (baudrate));

    return RESULT_SUCCESS;
}

/**
 * @brief Get the pointer to the base register address for the selected USART port
 * 
 * @param usart_no Port number
 * @return USART_TypeDef* The pointer to the base register address
 */
static USART_TypeDef * _get_usart_registers_struct(eDrvUsartNum_t usart_no)
{
    USART_TypeDef * usart = 0;

    switch (usart_no)
    {
        case DU_USART1: usart = USART1; break;
        case DU_USART2: usart = USART2; break;
        case DU_USART3: usart = USART3; break;
    }

    return usart;
}


/**
 * @brief Get USART Rx line status for new bytes
 * 
 * @param usart_no The number of USART port
 * @return uint32_t  1 if we have byte come
 */
uint32_t drv_usart_get_rx_status(eDrvUsartNum_t usart_no)
{
    volatile USART_TypeDef * usart = _get_usart_registers_struct(usart_no);
    return (usart->SR & USART_SR_RXNE);
}

/**
 * @brief Put char function
 * @todo  Make timeout for the while loop
 *
 * @param ch Char to output
 * @return none 
 */
void drv_usart_putc(eDrvUsartNum_t usart_no, uint8_t ch)
{
    volatile USART_TypeDef * usart = _get_usart_registers_struct(usart_no);
    usart->DR = ch;
    while (!(usart->SR & USART_SR_TXE));
}

/**
 * @brief Put string to the USART Tx line 
 * 
 * @param s The pointer to the string
 * @param len String length
 * @return Length that was output 
 */
int32_t drv_usart_puts(eDrvUsartNum_t usart_no, uint8_t *s, uint32_t len)
{
    uint32_t i;

    for (i = 0; i < len; i++)
    {
        drv_usart_putc(usart_no, s[i]);
    }

    return len;
}

/**
 * @brief Get char
 * 
 * @param usart_no Number of USART port that we should use to obtain byte
 * @param ch Pointer to the variable where to store the symbol
 * @return result_t The result of operation
 */
result_t drv_usart_getc(eDrvUsartNum_t usart_no, uint8_t * ch)
{
    volatile USART_TypeDef * usart = _get_usart_registers_struct(usart_no);
    result_t res = RESULT_NOTHING;

    if (drv_usart_get_rx_status(usart_no))
    {
        *ch = usart->DR; 
        res = RESULT_OK;
    }

    return res;
}