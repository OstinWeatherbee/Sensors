/**
 * @file drv_one_wire.c
 * @author Alexey Nikolaev (alexeynikzzz@gmail.com)
 * @brief 1-wire driver implementation for stm32f103xx series
 * @version 0.1
 * @date 2020-09-27
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#include "drv_one_wire.h"
#include "stm32f103xb.h"
#include "drv_usart.h"

/**
 * @brief Sends a bit to 1-Wire line
 * 
 * @param bit The bit to send
 */
void drv_one_wire_write_bit(uint8_t bit) 
{
    volatile uint8_t data;
    
    switch (bit) 
    {
      case 0: 
        data = 0x00;
        break;
      case 1: 
        data = 0xFF;
        break;
    }
    
    drv_usart_putc(DU_USART2, data);
    while(drv_usart_getc(DU_USART2, (uint8_t *)&data) != RESULT_OK && !(data == 0x00 || data == 0xFF));
}

/**
 * @brief Sends a byte to 1-Wire line
 * 
 * @param byte The byte to send
 */
void drv_one_wire_write_byte(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        drv_one_wire_write_bit((byte >> i) & 0x01);
    }
}

/**
 * @brief Sends data to 1-Wire line
 * 
 * @param data Pointer to the data to send
 * @param length The amount of data
 */
void drv_one_wire_write_data(uint8_t * data, uint8_t length)
{
    for (uint8_t i = 0; i < length; i++)
    {
        drv_one_wire_write_byte(*(data + i));
    }
}

/**
 * @brief Read bit from 1-Wire line
 * 
 * @return uint8_t Read bit
 */
uint8_t drv_one_wire_read_bit(void) 
{
    uint8_t bit;    
    volatile uint16_t counter = 0xFFFF;
    uint8_t read_usart_byte;

    drv_one_wire_write_bit(1);
    volatile result_t res = RESULT_NOTHING;
    while (counter-- > 0 && res == RESULT_NOTHING)
    {
        res = drv_usart_getc(DU_USART2, &read_usart_byte);
    }
        
    if (res != RESULT_OK)
    {
        DEBUG_PRINT("1-Wire: failed to read bit\r\n");
    }

    if (read_usart_byte < 0xFF)  
    {
        bit = 0;
    }
    else if (read_usart_byte == 0xFF)  
    {
        bit = 1;
    }
    
    return bit;
}

/**
 * @brief Read byte from 1-Wire line
 * 
 * @return uint8_t Read byte
 */
uint8_t drv_one_wire_read_byte(void) 
{
    uint8_t byte = 0;
    
    for (uint8_t i = 0; i < 8; i++)
    {
        byte |= drv_one_wire_read_bit() << i;
    }
    
    return byte;
}

/**
 * @brief Read the data from 1-Wire line
 * 
 * @param data Pointer to the place where to store read data
 * @param length The amount of bytes to read
 */
void drv_one_wire_read_data(uint8_t * data, uint8_t length) 
{
    for (uint8_t i = 0; i < length; i++)
    {
         *(data + i) = drv_one_wire_read_byte();
    }
}

/**
 * @brief Performs PRESENSE pulse on 1-Wire line (reset pulse)
 * 
 * @return result_t RESULT_OK if operation succeed
 */
result_t drv_one_wire_reset(void) 
{
    result_t res = RESULT_FAIL;

    xDrvUsartPortParams_t usart_params = {DU_USART2, DU_DATA_BITS_8, DU_STOP_BITS_1, DU_NO_PARITY, 9600};
    if (drv_usart_init_port(&usart_params) == RESULT_OK)
    {
        //USART2->DR;
        drv_usart_putc(DU_USART2, 0xF0);
        uint8_t read_usart_byte = 0;

        volatile result_t res_read = RESULT_NOTHING;
        while (res_read != RESULT_OK)
        {
            res_read = drv_usart_getc(DU_USART2, &read_usart_byte);
        }

        if (read_usart_byte < 0xF0)
        {
            //usart_params.baudrate = 115200;
            usart_params.baudrate = 120000;
            drv_usart_init_port(&usart_params);
            
            res = RESULT_OK;
        }
    }

    return res;
}