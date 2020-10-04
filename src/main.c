#include "stm32f1xx.h"
#include "drv_clocks.h"
#include "drv_usart.h"
#include "hal_ds18b20.h"
#include "macro.h"

#include "FreeRTOS.h"
#include "task.h"

// Dimensions the buffer that the task being created will use as its stack.
// NOTE:  This is the number of words the stack will hold, not the number of
// bytes.  For example, if each stack item is 32-bits, and this is set to 100,
// then 400 bytes (100 * 32-bits) will be allocated.
#define STACK_SIZE 200

// Structure that will hold the TCB of the task being created.
StaticTask_t xTaskBuffer;

// Buffer that the task being created will use as its stack.  Note this is
// an array of StackType_t variables.  The size of StackType_t is dependent on
// the RTOS port.
StackType_t xStack[ STACK_SIZE ];


StaticTask_t xGetTemperatureTaskBuffer;
StackType_t xGetTemperatureTaskStack[ STACK_SIZE ];

// Function that implements the task being created.
void vTaskCode( void * pvParameters )
{
    // The parameter value is expected to be 1 as 1 is passed in the
    // pvParameters value in the call to xTaskCreateStatic().
    configASSERT( ( uint32_t ) pvParameters == 1UL );

    for( ;; )
    {
        GPIOC->ODR &= ~GPIO_ODR_ODR13;		// Сбросили бит.
        vTaskDelay(pdMS_TO_TICKS(500));
        GPIOC->ODR |= GPIO_ODR_ODR13;		// Установили бит.
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Function that implements the task being created.
void vGetTemperatureTask( void * pvParameters )
{
    // The parameter value is expected to be 1 as 1 is passed in the
    // pvParameters value in the call to xTaskCreateStatic().
    configASSERT( ( uint32_t ) pvParameters == 1UL );

    static volatile hal_ds18b20_cxt_t sensor_cxt[5];
    result_t result = hal_ds18b20_init((hal_ds18b20_cxt_t *)sensor_cxt, ARRAY_SIZE(sensor_cxt));
    DEBUG_PRINT("DS18B20 init result: %d", result);

    for( ;; )
    {
        vTaskDelay(pdMS_TO_TICKS(5 *  1000));   //5 min

        hal_ds18b20_read_all_temperatures();

        PRINT("Temp:");
        for(uint8_t i = 0; i < ARRAY_SIZE(sensor_cxt), sensor_cxt[i].rom.qw; i++)
        {
            PRINT("\t%d. %.3f; ", i+1, sensor_cxt[i].temperature);
        }
        PRINT("\r\n");

        //DEBUG_PRINT("HELLO! temp: %.2f\r\n", temp);
    }
}

/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
/* If the buffers to be provided to the Idle task are declared inside this
function then they must be declared static – otherwise they will be allocated on
the stack and so not exists after this function exits. */
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task’s
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task’s stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*———————————————————–*/


int main(void)
{
    //printf("hi");
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN;
    GPIOC->CRH	&= ~GPIO_CRH_MODE13;	// Сбрасываем биты CNF для бита 5. Режим 00 - Push-Pull 
    GPIOC->CRH 	|= GPIO_CRH_MODE13_0;	// Выставляем бит MODE0 для пятого пина. Режим MODE01 = Max Speed 10MHz

    drv_clocks_init_sysclk();

    xDrvUsartPortParams_t usart1_params = {DU_USART1, DU_DATA_BITS_8, DU_STOP_BITS_1, DU_NO_PARITY, 115200};
    drv_usart_init_port(&usart1_params);

    TaskHandle_t xHandle = NULL;

    // Create the task without using any dynamic memory allocation.
    xHandle = xTaskCreateStatic(
                    vTaskCode,       // Function that implements the task.
                    "NAME",          // Text name for the task.
                    STACK_SIZE,      // Stack size in words, not bytes.
                    ( void * ) 1,    // Parameter passed into the task.
                    1,               // Priority at which the task is created.
                    xStack,          // Array to use as the task's stack.
                    &xTaskBuffer );  // Variable to hold the task's data structure.

    xTaskCreateStatic(
                    vGetTemperatureTask,       // Function that implements the task.
                    "TEMP",          // Text name for the task.
                    STACK_SIZE,      // Stack size in words, not bytes.
                    ( void * ) 1,    // Parameter passed into the task.
                    2,               // Priority at which the task is created.
                    xGetTemperatureTaskStack,          // Array to use as the task's stack.
                    &xGetTemperatureTaskBuffer );  // Variable to hold the task's data structure.

    // Start the scheduler.
    vTaskStartScheduler();

    while(1);
    
    return 1;
}


void _init(void)
{

}