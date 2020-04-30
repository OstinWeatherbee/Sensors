#include <stdint.h>
#include "stm32f1xx.h"

int main(void)
{
    //printf("hi");
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN;
    GPIOC->CRH	&= ~GPIO_CRH_MODE13;	// Сбрасываем биты CNF для бита 5. Режим 00 - Push-Pull 
    GPIOC->CRH 	|= GPIO_CRH_MODE13_0;	// Выставляем бит MODE0 для пятого пина. Режим MODE01 = Max Speed 10MHz

    while(1)
    {
        GPIOC->ODR &= ~GPIO_ODR_ODR13;		// Сбросили бит.
        for (int i = 0; i < 4000000; i++)
            asm("nop");			// Выдержка 600мс
        GPIOC->ODR |= GPIO_ODR_ODR13;		// Установили бит.
        for (int i = 0; i < 6000000; i++)
            asm("nop");			// Выдержка 600мс
    }

    return 1;
}


void _init(void)
{

}