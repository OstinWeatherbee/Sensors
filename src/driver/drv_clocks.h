/**
 * @file drv_clocks.h
 * @author Alexey Nikolaev (alexeynikzzz@gmail.com)
 * @brief Clocks driver implementation for stm32f103xx series
 * @version 0.1
 * @date 2020-05-30
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#ifndef _DRV_CLOCKS_
#define _DRV_CLOCKS_

#include "macro.h"

BOOL drv_clocks_init_sysclk(void);

/**
 * !!! NOT TESTED !!!
 * @brief Get PLL clock
 * 
 * @return The value of PLL clock
 */
uint32_t drv_clocks_get_pllclk(void);

/**
 * !!! NOT TESTED !!!
 * @brief Get system clock value
 * 
 * @return system clock value 
 */
uint32_t drv_clocks_get_sysclk(void);

/**
 * !!! NOT TESTED !!!
 * @brief Get AHB bus clock
 * 
 * @return AHB bus clock 
 */
uint32_t drv_clocks_get_hclk(void);

/**
 * !!! NOT TESTED !!!
 * @brief Get Secure digital input/output interface clock
 * 
 * @return Clock value 
 */
uint32_t drv_clocks_get_sdioclk(void);

/**
 * !!! NOT TESTED !!!
 * @brief Get Flexible static memory controller clock
 * 
 * @return Clock value 
 */
uint32_t drv_clocks_get_fsmcclk(void);

/**
 * !!! NOT TESTED !!!
 * @brief Get Cortex free running clock
 * 
 * @return Clock value 
 */
uint32_t drv_clocks_get_fclk(void);

/**
 * !!! NOT TESTED !!!
 * @brief Get Cortex system timer clock
 * 
 * @return Clock value 
 */
uint32_t drv_clocks_get_cortex_system_timer(void);

/**
 * !!! NOT TESTED !!!
 * @brief Get APB1 peripheral clock
 * 
 * @return Clock value 
 */
uint32_t drv_clocks_get_pclk1(void);

/**
 * !!! NOT TESTED !!!
 * @brief Get APB2 peripheral clock
 * 
 * @return Clock value 
 */
uint32_t drv_clocks_get_pclk2(void);

/**
 * !!! NOT TESTED !!!
 * @brief Get clock for specified clock
 * 
 * @param timer_no The number of the timer
 * @return Clock value 
 */
uint32_t drv_clocks_get_timxclk(uint32_t timer_no);

#endif  //_DRV_CLOCKS_