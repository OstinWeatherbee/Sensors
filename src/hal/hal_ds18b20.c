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

volatile static xScratchPad_t scratch_pad;

// global search state
volatile static struct xSearchRomCxt_t
{
    uint32_t last_discrepancy;
    uint32_t last_family_discrepancy;
    BOOL is_last_device;
    uint8_t crc8;
} _search_rom_cxt = {0};


// TEST BUILD
static unsigned char dscrc_table[] = {
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

//--------------------------------------------------------------------------
// Calculate the CRC8 of the byte value provided with the current 
// global 'crc8' value. 
// Returns current global crc8 value
//
unsigned char docrc8(unsigned char value)
{
   // See Application Note 27
   
   // TEST BUILD
   _search_rom_cxt.crc8 = dscrc_table[_search_rom_cxt.crc8 ^ value];
   return _search_rom_cxt.crc8;
}

static void _reset_search_rom_cxt(void);
static result_t _search_one_rom(volatile uint64_t * rom);

volatile static xRomType_t rom[5];
result_t hal_ds18b20_search_rom(void)
{
    uint8_t i = 0;
    while (_search_one_rom(&rom[i].qw) == RESULT_SUCCESS)
    {
        DEBUG_PRINT("Found: 0x%08x%08x", UPPER32(rom[i].qw), LOWER32(rom[i].qw));
    }
    return RESULT_OK;
}

static void _reset_search_rom_cxt(void)
{
    _search_rom_cxt.last_discrepancy = 0;
    _search_rom_cxt.is_last_device = FALSE;
    _search_rom_cxt.last_family_discrepancy = 0;
    _search_rom_cxt.crc8 = 0;
}

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

        // // if the search was successful then
        // if (crc8 == 0)
        // {
        //     search_result = RESULT_SUCCESS;
        // }
        //search_result = RESULT_SUCCESS;
    }

    // if no device found then reset counters so next 'search' will be like a first
    if (search_result != RESULT_SUCCESS || !*rom)
    {
        _reset_search_rom_cxt();
    }

   return search_result;
}


result_t hal_ds18b20_skip_rom(void) 
{
    drv_one_wire_reset();
    drv_one_wire_write_byte(CMD_SKIP_ROM);
    return RESULT_OK;
}

// result_t drv_one_wire_read_rom(RomType *addr) {
//     OW_UART_Reset();
//     OW_UART_Write_Byte(CMD_READ_ROM);
//     OW_UART_Read_Data((uint8_t *)addr, 9);
//     OW_UART_Reset();
    
//     if (Check_CRC8((*addr).b,9)) 
//         return OK;
//     else 
//         return FAULT;
// }


// void OW_UART_Read_Voltage(void) {
//     OW_UART_Convert_Voltage();
//     OW_UART_Recall_Memory(0);
//     OW_UART_Read_Scratch(0);
// }

void hal_ds18b20_recall_memory(uint8_t page) 
{
    hal_ds18b20_skip_rom();
    drv_one_wire_write_byte(CMD_RECALL_E2);
    drv_one_wire_write_byte(page);
}

result_t hal_ds18b20_read_scratch(uint8_t page)
{
    result_t res = RESULT_OK;
    hal_ds18b20_skip_rom();
    drv_one_wire_write_byte(CMD_READ_SCRATCHPAD);
    drv_one_wire_read_data((uint8_t *)(&(scratch_pad.page_0) + page), 9);
    
    // if (!Check_CRC8((uint8_t *)(&(scratch_pad.Page0) + page),9)) 
    // {
    //     res = RESULT_FAIL;
    // }
    
    return res;
}

void hal_ds18b20_convert_temperature(void) 
{
    hal_ds18b20_skip_rom();
    drv_one_wire_write_byte(CMD_CONVERT_TEMPERATURE);
    while(!drv_one_wire_read_bit());
}

float hal_ds18b20_read_temperature(void) 
{
    hal_ds18b20_convert_temperature();
    //hal_ds18b20_recall_memory(0);
    hal_ds18b20_read_scratch(0);

    
    float temp = (float)(scratch_pad.page_0.temp_msb << 4 | scratch_pad.page_0.temp_lsb >> 4) + (scratch_pad.page_0.temp_lsb & 0xF) * 0.0625;
    return temp;
}
