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

