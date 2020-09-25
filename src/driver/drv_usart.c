/**
 * @file drv_usart.c
 * @author Alexey Nikolaev (alexeynikzzz@gmail.com)
 * @brief USART driver implementation for stm32f103xx series
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

volatile static xDrvUsartPortParams_t ports[DU_USART_NUM];

result_t _calculate_usartdiv_for_baudrate(xDrvUsartPortParams_t * port_params, uint32_t * usart_div);

result_t drv_usart_init_port(xDrvUsartPortParams_t * port_params)
{
    result_t res = RESULT_OK;

    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;

    //TX pin
    GPIOA->CRH &= ~(GPIO_CRH_MODE9 | GPIO_CRH_CNF9); //Clear PA9 configuration
    GPIOA->CRH |= GPIO_CRH_MODE9_1 | GPIO_CRH_CNF9_1; //Set PA9 to output 2MHz, alternate push-pull

    //RX pin
    GPIOA->CRH &= ~(GPIO_CRH_MODE10 | GPIO_CRH_CNF10); //Clear PA10 configuration
    GPIOA->CRH |= GPIO_CRH_CNF10_0; //Set PA10 to floating input
    //GPIOA->BSRR |= GPIO_ODR_ODR10;
    
    USART1->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;   //Enable USART with Rx and Tx lines
    uint32_t usart_div;
    _calculate_usartdiv_for_baudrate(port_params, &usart_div);
    USART1->BRR = usart_div;    

    USART1->CR1 |= USART_CR1_RXNEIE;

    return res;
}

/**
 * @brief USART initialization
 * 
 * @return result_t 
 */
result_t drv_usart1_init(void)
{
    result_t res = RESULT_OK;

    GPIOA->CRH &= ~(GPIO_CRH_MODE9 | GPIO_CRH_CNF9); //Clear PA9 configuration
    GPIOA->CRH |= GPIO_CRH_MODE9_1 | GPIO_CRH_CNF9_1; //Set PA9 to output 2MHz, alternate push-pull
    GPIOA->CRH &= ~(GPIO_CRH_MODE10 | GPIO_CRH_CNF10); //Clear PA10 configuration
    GPIOA->CRH |= GPIO_CRH_CNF10_0; //Set PA10 to floating input
    
    USART1->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;   //Enable USART with Rx and Tx lines
    //uint32_t usart_div;
    //_calculate_usartdiv_for_baudrate(&usart_div);
    //USART1->BRR = usart_div;


    return res;
}

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

result_t _calculate_usartdiv_for_baudrate(xDrvUsartPortParams_t * port_params, uint32_t * usart_div)
{
    eDrvUsartNum_t usart_no = port_params->usart_num;
    uint32_t baudrate = port_params->baudrate;
    uint32_t frequency = drv_usart_get_clock(usart_no);

    *usart_div = (uint32_t) roundf((float) frequency / (float) (baudrate));

    return RESULT_SUCCESS;
}
