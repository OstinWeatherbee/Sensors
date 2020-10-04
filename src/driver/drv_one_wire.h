/**
 * @file drv_one_wire.h
 * @author Alexey Nikolaev (alexeynikzzz@gmail.com)
 * @brief 1-wire driver implementation for stm32f103xx series
 * @version 0.1
 * @date 2020-09-27
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#ifndef _DRV_ONE_WIRE_
#define _DRV_ONE_WIRE_

#include "types.h"
#include "stm32f103xb.h"

void drv_one_wire_write_bit(uint8_t bit);
void drv_one_wire_write_byte(uint8_t byte);
void drv_one_wire_write_data(uint8_t * data, uint8_t length);
uint8_t drv_one_wire_read_bit(void);
uint8_t drv_one_wire_read_byte(void);
void drv_one_wire_read_data(uint8_t * data, uint8_t length);
result_t drv_one_wire_reset(void);

 

#endif  //_DRV_ONE_WIRE_