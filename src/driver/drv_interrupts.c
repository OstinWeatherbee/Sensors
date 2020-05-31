/**
 * @file drv_interrupts.c
 * @author Alexey Nikolaev (alexeynikzzz@gmail.com)
 * @brief Interrupt driver implementation for stm32f103xx series
 * @version 0.1
 * @date 2020-05-30
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#include "drv_interrupts.h"
#include "port.h"

void NMI_Handler(void)
{
    while(1);
}

void HardFault_Handler(void)
{
    while(1);

}


void SVC_Handler(void)
{
    vPortSVCHandler();
}


void PendSV_Handler(void)
{
    xPortPendSVHandler();
}

void SysTick_Handler(void)
{
    xPortSysTickHandler();
}