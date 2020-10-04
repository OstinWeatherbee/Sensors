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

#define DS18B20_INVALID_ROM         0xFFFFFFFFFFFFFFFFULL
#define DS18B20_BROADCAST_ROM       0xFFFFFFFFFFFFFFFFULL

typedef union {
	uint64_t qw;
	uint8_t b[8];
	struct {
		uint8_t family;
		uint8_t addr[6];
		uint8_t crc;
	};
} hal_ds18b20_rom_t;

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
} hal_ds18b20_scratch_pad_t;

typedef struct 
{
    hal_ds18b20_rom_t rom;
    float temperature;
} hal_ds18b20_cxt_t;


/**
 * @brief Initialisation of the sensors' context, interface and obtaining of 
 *      devices' ROM
 * 
 * @param cxt[in/out] Pointer to the context we should initialize with found devices
 * @param size[in] The size of array of the context
 * @return result_t RESULT_OK if initialization succeed
 */
result_t hal_ds18b20_init(hal_ds18b20_cxt_t * cxt, uint32_t size);

/**
 * @brief This function obtains ROM code of all devices on the 1-Wire line
 * 
 * @return result_t RESULT_OK if we found all devices without troubles
 */
result_t hal_ds18b20_search_rom(void);

/**
 * @brief We can read ROM in case if we have only one device on the line
 * 
 * @param rom[out] Sensor's ROM 
 * @return result_t RESULT_OK if we succeed to get ROM
 */
result_t hal_ds18b20_read_rom(hal_ds18b20_rom_t * rom);

/**
 * @brief Before we send any command to the sensor we should choose it
 *      by this command
 * 
 * @param rom[in] Sensor's ROM
 * @return result_t RESULT_OK if the command was sent successfully
 */
result_t hal_ds18b20_match_rom(hal_ds18b20_rom_t * rom);

/**
 * @brief Skip ROM command allows to send broadcast commands or to save time
 *      for the command in case  we have only one device on the line
 * 
 * @return result_t RESULT_OK if the command was sent successfully
 */
result_t hal_ds18b20_skip_rom(void);

void hal_ds18b20_recall_memory(uint8_t page);

/**
 * @brief Reads scratch pad of the sensor
 * 
 * @param rom[in] The device's ROM
 * @param scratch[out] The struct where to store scratch pad values
 * @param page[in] The number of scratch pad page 
 * @return result_t RESULT_OK if we read out successfully
 */
result_t hal_ds18b20_read_scratch(hal_ds18b20_rom_t * rom, hal_ds18b20_scratch_pad_t * scratch, uint8_t page);

/**
 * @brief Convert temperature function sends the command and waits till conversion ends
 * 
 * @param rom[in] The device we should start conversion
 * @return result_t RESULT_OK if conversion finished successfully
 */
result_t hal_ds18b20_convert_temperature(hal_ds18b20_rom_t * rom);

/**
 * @brief Read temperature from the one sensor with matching ROM
 * 
 * @param rom[in] Sensors ROM
 * @param temperature[out] Temperature value
 * @return result_t RESULT_OK if temperature was read out
 */
result_t hal_ds18b20_read_temperature(hal_ds18b20_rom_t * rom, float * temperature);

/**
 * @brief Read out temperature values from all sensors connected to the 1-Wire interface
 * 
 * @return result_t RESULT_OK if all temperatures were read out
 */
result_t hal_ds18b20_read_all_temperatures(void);

#endif  //_HAL_DS18B20_