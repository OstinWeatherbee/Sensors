/**
 * @file drv_clocks.c
 * @author Alexey Nikolaev (alexeynikzzz@gmail.com)
 * @brief Clocks driver implementation for stm32f103xx series
 * @version 0.1
 * @date 2020-05-30
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#include "drv_clocks.h"
#include "stm32f1xx.h"
#include "config.h"


BOOL drv_clocks_init_sysclk(void)
{
    // Choose HSE as clock source
    RCC->CR |= RCC_CR_HSEON;
    // Wait for HSE ready
    while (!(RCC->CR & RCC_CR_HSERDY));

    // Choose HSE as PLL source
    RCC->CFGR |= RCC_CFGR_PLLSRC;
    RCC->CFGR |= RCC_CFGR_PLLMULL_0 | RCC_CFGR_PLLMULL_1 | RCC_CFGR_PLLMULL_2;  //0111 PLL input clock x9

    //Turn on PLL and wait for ready state
    RCC->CR |= RCC_CR_PLLON;
    while(!(RCC->CR & RCC_CR_PLLRDY));

    // Flash memory cannot work on frequencies larger than 24 MHz so we'll add some latency
    FLASH->ACR |= (0x02<<FLASH_ACR_LATENCY_Pos);

    //Turn off AHB prescaler
    RCC->CFGR &= ~RCC_CFGR_HPRE_3;  

    // Connect PLL output to system clock and wait while ready
    RCC->CFGR &= ~RCC_CFGR_SW_0;
    RCC->CFGR |= RCC_CFGR_SW_1;
    while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_1);

    return TRUE;
}

/**
 * @brief Get multiplier for PLL clock output
 * 
 * @return The value of PLL multiplier
 */
INLINE uint32_t _get_pllmul(void)
{
    uint32_t mul = 0;
    uint32_t reg = (RCC->CFGR & RCC_CFGR_PLLMULL) >> RCC_CFGR_PLLMULL_Pos;

    if (reg >= 0x0 && reg <= 0xE)
    {
        mul = reg + 2;
    }
    else if (reg == 0xF)
    {
        mul = 16;
    }

    return mul;
}


/**
 * @brief Get PLL clock
 * 
 * @return The value of PLL clock
 */
uint32_t drv_clocks_get_pllclk(void)
{
    uint32_t pllsrc = 0;

    if (RCC->CFGR & RCC_CFGR_PLLSRC)
    {
        if (RCC->CFGR & RCC_CFGR_PLLXTPRE)
        {
            pllsrc = HSE_FREQUENCY / 2;
        }
        else
        {
            pllsrc = HSE_FREQUENCY;
        }
    }
    else
    {
        pllsrc = HSI_FREQUENCY / 2;
    }
    
    return pllsrc * _get_pllmul();
}

/**
 * @brief Get system clock value
 * 
 * @return system clock value 
 */
uint32_t drv_clocks_get_sysclk(void)
{
    uint32_t sysclk = 0;

    //Get sysclk by its source
    switch (RCC->CFGR & RCC_CFGR_SW)
    {
        case RCC_CFGR_SW_HSI:
            sysclk = HSI_FREQUENCY;
            break;
        case RCC_CFGR_SW_HSE:
            sysclk = HSE_FREQUENCY;
            break;
        case RCC_CFGR_SW_PLL:
            sysclk = drv_clocks_get_pllclk();
            break;
    }

    return sysclk;
}

/**
 * @brief Get AHB clock prescaler
 * 
 * @return AHB clock prescaler 
 */
INLINE uint32_t _get_ahb_prescaler(void)
{
    uint32_t ahb_prescaler = 1;
    uint32_t hpre = (RCC->CFGR & RCC_CFGR_HPRE) >> RCC_CFGR_HPRE_Pos;

    if (hpre >= 0x8 && hpre <= 0xB)
    {
        ahb_prescaler = 1 << (hpre - 7);
    }
    else if (hpre >= 0xC && hpre <= 0xF)
    {
        ahb_prescaler = 1 << (hpre - 6);
    }    

    return ahb_prescaler;
}

/**
 * @brief Get APB1 peripheral clock prescaler
 * 
 * @return APB1 peripheral clock prescaler 
 */
INLINE uint32_t _get_apb1_prescaler(void)
{
    uint32_t apb1_prescaler = 1;
    uint32_t ppre1 = (RCC->CFGR & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos;

    if (ppre1 >= 0x4 && ppre1 <= 0x7)
    {
        apb1_prescaler = 1 << (ppre1 - 3);
    }

    return apb1_prescaler;
}

/**
 * @brief Get APB2 peripheral clock prescaler
 * 
 * @return APB2 peripheral clock prescaler 
 */
INLINE uint32_t _get_apb2_prescaler(void)
{
    uint32_t apb2_prescaler = 1;
    uint32_t ppre2 = (RCC->CFGR & RCC_CFGR_PPRE2) >> RCC_CFGR_PPRE2_Pos;

    if (ppre2 >= 0x4 && ppre2 <= 0x7)
    {
        apb2_prescaler = 1 << (ppre2 - 3);
    }

    return apb2_prescaler;
}

/**
 * @brief Get AHB bus clock
 * 
 * @return AHB bus clock 
 */
uint32_t drv_clocks_get_hclk(void)
{
    return drv_clocks_get_sysclk() / _get_ahb_prescaler();
}

/**
 * @brief Get Secure digital input/output interface clock
 * 
 * @return Clock value 
 */
uint32_t drv_clocks_get_sdioclk(void)
{
    return drv_clocks_get_hclk();
}

/**
 * @brief Get Flexible static memory controller clock
 * 
 * @return Clock value 
 */
uint32_t drv_clocks_get_fsmcclk(void)
{
    return drv_clocks_get_hclk();
}

/**
 * @brief Get Cortex free running clock
 * 
 * @return Clock value 
 */
uint32_t drv_clocks_get_fclk(void)
{
    return drv_clocks_get_hclk();
}

/**
 * @brief Get Cortex system timer clock
 * 
 * @return Clock value 
 */
uint32_t drv_clocks_get_cortex_system_timer(void)
{
    return drv_clocks_get_hclk() / 8;
}

/**
 * @brief Get APB1 peripheral clock
 * 
 * @return Clock value 
 */
uint32_t drv_clocks_get_pclk1(void)
{
    return drv_clocks_get_hclk() / _get_apb1_prescaler();
}

/**
 * @brief Get APB2 peripheral clock
 * 
 * @return Clock value 
 */
uint32_t drv_clocks_get_pclk2(void)
{
    return drv_clocks_get_hclk() / _get_apb2_prescaler();
}

/**
 * @brief Get clock for specified clock
 * 
 * @param timer_no The number of the timer
 * @return Clock value 
 */
uint32_t drv_clocks_get_timxclk(uint32_t timer_no)
{
    uint32_t timxclk = 0;

    switch (timer_no)
    {
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 12:
        case 13:
        case 14:
            if (_get_apb1_prescaler() == 1)
            {
                timxclk = drv_clocks_get_pclk1();
            }
            else
            {
                timxclk = drv_clocks_get_pclk1() * 2;
            }
            break;
        case 1:
        case 8:
        case 9:
        case 10:
        case 11:
            if (_get_apb2_prescaler() == 1)
            {
                timxclk = drv_clocks_get_pclk2();
            }
            else
            {
                timxclk = drv_clocks_get_pclk2() * 2;
            }
            break;
    }

    return timxclk;
}