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
#ifndef _HAL_DS18B20_
#define _HAL_DS18B20_

#include "types.h"
#include  <stdint.h>

#define CMD_READ_ROM                0x33
#define CMD_CONVERT_TEMPERATURE     0x44
#define CMD_MATCH_ROM               0x55
#define CMD_SKIP_ROM                0xCC
#define CMD_ALARM_SEARCH            0xEC
#define CMD_SEARCH_ROM              0xF0

#define CMD_WRITE_SCRATCHPAD        0x4E
#define CMD_READ_SCRATCHPAD         0xBE
#define CMD_COPY_SCRATCHPAD         0x48
#define CMD_RECALL_E2               0xB8
#define CMD_READ_POWER_SUPPLY       0xB4

typedef union {
	uint64_t qw;
	uint8_t b[8];
	struct {
		uint8_t family;
		uint8_t addr[6];
		uint8_t crc;
	};
} xRomType_t;

typedef struct  {
    struct  {
        uint8_t temp_lsb;
        uint8_t temp_msb;
        union 
        {
            uint8_t th_register;
            uint8_t user_byte_1;
        };
        union 
        {
            uint8_t tl_register;
            uint8_t user_byte_2;
        };
        uint8_t configuration_register;
        uint8_t reserved[3];
        uint8_t crc;
    } page_0;
} xScratchPad_t;

result_t hal_ds18b20_search_rom(void);
result_t hal_ds18b20_skip_rom(void);
void hal_ds18b20_recall_memory(uint8_t page);
void hal_ds18b20_convert_temperature(void);
float hal_ds18b20_read_temperature(void);


#endif  //_HAL_DS18B20_