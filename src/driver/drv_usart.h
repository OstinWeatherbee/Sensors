/**
 * @file drv_usart.h
 * @author Alexey Nikolaev (alexeynikzzz@gmail.com)
 * @brief USART driver implementation for stm32f103xx series
 * @version 0.1
 * @date 2020-04-26
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#ifndef _DRV_USART_
#define _DRV_USART_

#include "types.h"
#include "stm32f103xb.h"

typedef enum
{
    DU_USART1,
    DU_USART2,
    DU_USART3,
    DU_USART4,
    DU_USART5,

    DU_USART_NUM
} eDrvUsartNum_t;

typedef enum
{
    DU_DATA_BITS_8      = 8,
    DU_DATA_BITS_9      = 9
} eDrvUsartDataBits_t;

typedef enum
{
    DU_STOP_BITS_0_5    = 5,
    DU_STOP_BITS_1      = 10,
    DU_STOP_BITS_1_5    = 15,
    DU_STOP_BITS_2      = 20,
} eDrvUsartStopBits_t;

typedef enum
{
    DU_NO_PARITY,
    DU_PARITY_EVEN,
    DU_PARITY_ODD
} eDrvUsartParity_t;


typedef struct
{
    eDrvUsartNum_t      usart_num;
    eDrvUsartDataBits_t data_bits;
    eDrvUsartStopBits_t stop_bits;
    eDrvUsartParity_t   parity;
    uint32_t            baudrate;
} xDrvUsartPortParams_t;

//result_t drv_usart_init(void);
result_t drv_usart_init_port(xDrvUsartPortParams_t * port_params);

#endif  //_DRV_USART_