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
#include "hal_ds18b20.h"
#include "drv_one_wire.h"
#include "macro.h"
#include <string.h>
#include <stdlib.h>

static struct 
{
    hal_ds18b20_cxt_t * ptr; 
    uint32_t size;
} _cxt;

/**
 * @brief Initialisation of the sensors' context, interface and obtaining of 
 *      devices' ROM
 * 
 * @param cxt[in/out] Pointer to the context we should initialize with found devices
 * @param size[in] The size of array of the context
 * @return result_t RESULT_OK if initialization succeed
 */
result_t hal_ds18b20_init(hal_ds18b20_cxt_t * cxt, uint32_t size)
{
    result_t result = RESULT_OK;

    if (!cxt || !size)
    {
        result = RESULT_FAIL;
    }
    else
    {
        _cxt.ptr =  cxt;
        _cxt.size = size; 
        memset((void*)cxt, 0, sizeof(hal_ds18b20_cxt_t) * size);
        result = hal_ds18b20_search_rom();
    }

    return result;
}



static uint8_t crc_table[] = {
        0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
      157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
       35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
      190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
       70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
      219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
      101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
      248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
      140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
       17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
      175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
       50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
      202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
       87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
      233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
      116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53};


/**
 * @brief Calculate crc8 for DS18B20 sensor
 * 
 * @param message[in] Pointer to the data to check
 * @param length[in] Data length
 * @return uint8_t The calculated value
 */
static uint8_t _calculate_crc8(uint8_t const * message, uint32_t length)
{
    uint8_t data;
    uint8_t remainder = 0;

    for (int byte = 0; byte < length; ++byte)
    {
        data = message[byte] ^ remainder;
        remainder = crc_table[data] ^ (remainder << 8);
    }

    return remainder;
}

// rom search state
volatile static struct xSearchRomCxt_t
{
    uint32_t last_discrepancy;
    uint32_t last_family_discrepancy;
    BOOL is_last_device;
    uint8_t crc8;
} _search_rom_cxt = {0};

/**
 * @brief Reset ROM search context
 * 
 */
static void _reset_search_rom_cxt(void)
{
    _search_rom_cxt.last_discrepancy = 0;
    _search_rom_cxt.is_last_device = FALSE;
    _search_rom_cxt.last_family_discrepancy = 0;
    _search_rom_cxt.crc8 = 0;
}

/**
 * @brief Search ROM of one device
 * 
 * @param rom[out] The pointer to the ROM storage
 * @return result_t RESULT_OK if we found one more ROM
 */
static result_t _search_one_rom( volatile uint64_t * rom)
{
   volatile uint8_t id_bit_number = 0;
   volatile uint8_t last_zero = 0;
   result_t search_result = RESULT_FAIL;
   volatile uint8_t id_bit, cmp_id_bit;
   volatile uint8_t search_direction;

    // 1-Wire reset
    if (drv_one_wire_reset() != RESULT_OK)
    {
        _reset_search_rom_cxt();
        DEBUG_PRINT("Search ROM reset failed");
        return RESULT_FAIL;
    }

    // if the last call was not the last one
   if (!_search_rom_cxt.is_last_device)
   {

        // issue the search command 
        drv_one_wire_write_byte(CMD_SEARCH_ROM); 

        // loop to do the search
        do
        {
            // read a bit and its complement
            id_bit = drv_one_wire_read_bit();
            cmp_id_bit = drv_one_wire_read_bit();

            // check for no devices on 1-wire
            if ((id_bit == 1) && (cmp_id_bit == 1))
            {
                DEBUG_PRINT("Search ROM line error");
                break;
            }
            else
            {

                // all devices coupled have 0 or 1
                if (id_bit == 0 && cmp_id_bit == 0)
                {
                    if (id_bit_number == _search_rom_cxt.last_discrepancy)
                    {
                        search_direction = 1;
                    }
                    else
                    {
                        if (id_bit_number > _search_rom_cxt.last_discrepancy)
                        {
                            search_direction = 0;
                        }
                        else
                        {
                            search_direction = (*rom >> id_bit_number) & 0x1;
                        }
                        
                    }
                    

                    // if 0 was picked then record its position in LastZero
                    if (search_direction == 0)
                    {
                        last_zero = id_bit_number;

                        // check for Last discrepancy in family
                        if (last_zero < 9)
                        {
                            _search_rom_cxt.last_family_discrepancy = last_zero;
                        }
                    }
                }
                else
                {
                    search_direction = id_bit;  // bit write value for search
                }

                // set or clear the bit in the ROM byte rom_byte_number
                // with mask rom_byte_mask
                uint64_t bit_mask = 1ULL << id_bit_number;

                if (search_direction == 1)
                {
                    *rom |= bit_mask;
                }
                else
                {
                    *rom &= ~bit_mask;
                }

                //DEBUG_PRINT("write search dir: %d", search_direction);

                // serial number search direction write bit
                drv_one_wire_write_bit(search_direction);

                // increment the byte counter id_bit_number
                id_bit_number++;
            }
        } while(id_bit_number < 64);  // loop until through all ROM bytes 0-7

        // search successful so set last_discrepancy,is_last_device,search_result
        _search_rom_cxt.last_discrepancy = last_zero;

        // check for last device
        if (_search_rom_cxt.last_discrepancy == 0)
        {
            _search_rom_cxt.is_last_device = TRUE;
        }
      
        if (id_bit_number == 64)
        {
            search_result = RESULT_SUCCESS;
        }

        // if the search was successful then
        if (_calculate_crc8((uint8_t *) rom, sizeof(uint64_t)) == 0)
        {
            search_result = RESULT_SUCCESS;
        }
    }

    // if no device found then reset counters so next 'search' will be like a first
    if (search_result != RESULT_SUCCESS || !*rom)
    {
        _reset_search_rom_cxt();
    }

   return search_result;
}

/**
 * @brief This function obtains ROM code of all devices on the 1-Wire line
 * 
 * @return result_t RESULT_OK if we found all devices without troubles
 */
result_t hal_ds18b20_search_rom(void)
{
    uint8_t i = 0;
    while (_search_one_rom(&_cxt.ptr[i].rom.qw) == RESULT_SUCCESS && i < _cxt.size)
    {
        DEBUG_PRINT("Found: 0x%08x%08x", UPPER32(_cxt.ptr[i].rom.qw), LOWER32(_cxt.ptr[i].rom.qw));
        i++;
    }
    return RESULT_OK;
}

/**
 * @brief We can read ROM in case if we have only one device on the line
 * 
 * @param rom[out] Sensor's ROM 
 * @return result_t RESULT_OK if we succeed to get ROM
 */
result_t hal_ds18b20_read_rom(hal_ds18b20_rom_t * rom) 
{
    drv_one_wire_reset();
    drv_one_wire_write_byte(CMD_READ_ROM);
    drv_one_wire_read_data((uint8_t *) rom, sizeof(uint64_t));
    return RESULT_OK;
}

/**
 * @brief Before we send any command to the sensor we should choose it
 *      by this command
 * 
 * @param rom[in] Sensor's ROM
 * @return result_t RESULT_OK if the command was sent successfully
 */
result_t hal_ds18b20_match_rom(hal_ds18b20_rom_t * rom) 
{
    drv_one_wire_reset();
    drv_one_wire_write_byte(CMD_MATCH_ROM);
    drv_one_wire_write_data((uint8_t *) rom, sizeof(uint64_t));
    return RESULT_OK;
}

/**
 * @brief Skip ROM command allows to send broadcast commands or to save time
 *      for the command in case  we have only one device on the line
 * 
 * @return result_t RESULT_OK if the command was sent successfully
 */
result_t hal_ds18b20_skip_rom(void) 
{
    drv_one_wire_reset();
    drv_one_wire_write_byte(CMD_SKIP_ROM);
    return RESULT_OK;
}


void hal_ds18b20_recall_memory(uint8_t page) 
{
    hal_ds18b20_skip_rom();
    drv_one_wire_write_byte(CMD_RECALL_E2);
    drv_one_wire_write_byte(page);
}

/**
 * @brief Reads scratch pad of the sensor
 * 
 * @param rom[in] The device's ROM
 * @param scratch[out] The struct where to store scratch pad values
 * @param page[in] The number of scratch pad page 
 * @return result_t RESULT_OK if we read out successfully
 */
result_t hal_ds18b20_read_scratch(hal_ds18b20_rom_t * rom, hal_ds18b20_scratch_pad_t * scratch, uint8_t page)
{
    result_t res = RESULT_OK;
    hal_ds18b20_match_rom(rom);
    drv_one_wire_write_byte(CMD_READ_SCRATCHPAD);
    drv_one_wire_read_data((uint8_t *)(&(scratch->page_0) + page), 9);
    
    if (_calculate_crc8((uint8_t *)(&(scratch->page_0) + page),9) != 0) 
    {
        res = RESULT_FAIL;
    }
    
    return res;
}

/**
 * @brief Convert temperature function sends the command and waits till conversion ends
 * 
 * @param rom[in] The device we should start conversion
 * @return result_t RESULT_OK if conversion finished successfully
 */
result_t hal_ds18b20_convert_temperature(hal_ds18b20_rom_t * rom) 
{
    result_t result;
    if (rom->qw == DS18B20_BROADCAST_ROM)
    {
        result = hal_ds18b20_skip_rom();
    }
    else
    {
        result = hal_ds18b20_match_rom(rom);
    }
    
    drv_one_wire_write_byte(CMD_CONVERT_TEMPERATURE);
    while(!drv_one_wire_read_bit());

    return result;
}

/**
 * @brief Read temperature from the one sensor with matching ROM
 * 
 * @param rom[in] Sensors ROM
 * @param temperature[out] Temperature value
 * @return result_t RESULT_OK if temperature was read out
 */
result_t hal_ds18b20_read_temperature(hal_ds18b20_rom_t * rom, float * temperature) 
{
    result_t result = RESULT_OK;
    volatile static hal_ds18b20_scratch_pad_t scratch;

    result = hal_ds18b20_convert_temperature(rom);
    if (result != RESULT_OK)
    {
        return result;
    }

    result = hal_ds18b20_read_scratch(rom, (hal_ds18b20_scratch_pad_t *)&scratch, 0);
    if (result != RESULT_OK)
    {
        return result;
    }

    int32_t sign = scratch.page_0.temp_msb & 0x8 ? 0xFFFF<<16 : 0;
    volatile int32_t int_part = (int32_t)(scratch.page_0.temp_msb << 8 | scratch.page_0.temp_lsb);
    int_part |= sign;
    *temperature = int_part / 16.0;

    return result;
}

/**
 * @brief Read out temperature values from all sensors connected to the 1-Wire interface
 * 
 * @return result_t RESULT_OK if all temperatures were read out
 */
result_t hal_ds18b20_read_all_temperatures(void)
{
    for (uint8_t i = 0; i < _cxt.size; i++)
    {
        if (_cxt.ptr[i].rom.qw)
        {
            volatile float temp;
            if (hal_ds18b20_read_temperature((hal_ds18b20_rom_t *)&_cxt.ptr[i].rom, (float *)&temp) == RESULT_OK)
            {
                _cxt.ptr[i].temperature = temp;
            }
            else
            {
                _cxt.ptr[i].temperature = -273.0;
            }
        }
        else
        {
            break;
        }
        
    }

    return RESULT_OK;
}