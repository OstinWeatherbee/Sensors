/*
 * The Minimal snprintf() implementation
 *
 * Copyright (c) 2013,2014 Michal Ludvig <michal@logix.cz>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the auhor nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ----
 *
 * This is a minimal snprintf() implementation optimised
 * for embedded systems with a very limited program memory.
 * mini_snprintf() doesn't support _all_ the formatting
 * the glibc does but on the other hand is a lot smaller.
 * Here are some numbers from my STM32 project (.bin file size):
 *      no snprintf():      10768 bytes
 *      mini snprintf():    11420 bytes     (+  652 bytes)
 *      glibc snprintf():   34860 bytes     (+24092 bytes)
 * Wasting nearly 24kB of memory just for snprintf() on
 * a chip with 32kB flash is crazy. Use mini_snprintf() instead.
 *
 */

#include "mini-printf.h"
#include "drv_usart.h"
#include "math.h"
#include "macro.h"

//TODO: Need to check this function
static int32_t mini_putc(uint8_t * const ptr, const uint8_t ch)
{
    *ptr = ch;
    
    return 0;
}

//TODO: Need to check this function
static int32_t mini_puts(const uint8_t * const ptr_in, const uint32_t len_in, uint8_t * const ptr_out)
{
    uint32_t i;

    for (i = 0; i < len_in; i++)
    {
        mini_putc(ptr_out + i, ptr_in[i]);
    }
    mini_putc(ptr_out + i,'\0');

    return len_in;
}

static uint32_t mini_strlen(const uint8_t *s)
{
    uint32_t len = 0;
    while (s[len] != '\0') 
        len++;
    return len;
}

static uint32_t mini_itoa(int32_t value, uint32_t radix, uint32_t uppercase, uint32_t unsig,
     uint8_t *buffer, uint32_t zero_pad)
{
    uint8_t    *pbuffer = buffer;
    int32_t    negative = 0;
    int32_t    i, len;

    /* No support for unusual radixes. */
    if (radix > 16)
        return 0;

    if (value < 0 && !unsig) {
        negative = 1;
        value = -value;
    }

    /* This builds the string back to front ... */
    do {
        int digit = value % radix;
        *(pbuffer++) = (digit < 10 ? '0' + digit : (uppercase ? 'A' : 'a') + digit - 10);
        value /= radix;
    } while (value > 0);

    for (i = (pbuffer - buffer); i < zero_pad; i++)
        *(pbuffer++) = '0';

    if (negative)
        *(pbuffer++) = '-';

    *(pbuffer) = '\0';

    /* ... now we reverse it (could do it recursively but will
     * conserve the stack space) */
    len = (pbuffer - buffer);
    for (i = 0; i < len / 2; i++) {
        char j = buffer[i];
        buffer[i] = buffer[len-i-1];
        buffer[len-i-1] = j;
    }

    return len;
}


static uint32_t mini_ftoa(float value, uint32_t uppercase, uint8_t *buffer, uint32_t width, uint32_t precision, uint8_t is_zero_pad)
{
    uint8_t    *pbuffer = buffer;
    int32_t    i, len, negative = 0;
    float saved_value;
    
    if (value < 0.0) 
    {
        negative = 1;
        value = -value;
    }
    saved_value = value;

    /* This builds the string back to front ... */
    if (precision)
    {
        uint32_t precision_len = 0;
        do 
        {
            uint32_t digit = (uint32_t)(value * (pow(10, precision - precision_len))) % 10;
            *(pbuffer++) = '0' + digit;
            precision_len++;
        } while (precision_len < precision);

        *(pbuffer++) = '.';
    }

    uint32_t width_len = 0;
    value = saved_value;
    do 
    {
        uint32_t digit = (uint32_t)value % 10;
        *(pbuffer++) = '0' + digit;
        width_len++;
        value /= 10;
    } while ((value > 1.0) &&  (width > 0 ? (width_len < width) : 1 ));


    for (i = (pbuffer - buffer - precision - (precision ? 1 : 0)); i < width; i++)
    {
        if (is_zero_pad)
        {
            *(pbuffer++) = '0';
        }
        else
        {
            *(pbuffer++) = ' ';
        }
    }

    if (negative)
        *(pbuffer++) = '-';

    *(pbuffer) = '\0';

    /* ... now we reverse it (could do it recursively but will
     * conserve the stack space) */
    len = (pbuffer - buffer);
    for (i = 0; i < len / 2; i++) {
        char j = buffer[i];
        buffer[i] = buffer[len-i-1];
        buffer[len-i-1] = j;
    }

    return len;
}

struct mini_buff {
    uint8_t *buffer, *pbuffer;
    uint32_t buffer_len;
};

int32_t mini_printf(const uint8_t *fmt, ...)
{
    va_list arglist;
    uint8_t bf[24];
    uint8_t ch;

    va_start( arglist, fmt );
    while ((ch=*(fmt++))) 
    {
        if (ch!='%')
        {
            drv_usart_putc(DU_USART1, ch);
        }
        else 
        {
            BOOL is_zero_pad = FALSE;
            //volatile BOOL is_force_positive = FALSE;
            uint32_t width = 0;
            uint32_t precision = 0;
            uint8_t *ptr;
            uint32_t len;

            struct
            {
                uint8_t flags : 1;
                uint8_t width : 1;
                uint8_t precision : 1;
                uint8_t length : 1;
            } fmt_sub_specifiers = {0};

            ch=*(fmt++);

            /* Flags */
            fmt_sub_specifiers.flags = 1;
            switch(ch)
            {
                case '-':
                    break;
                case '+':
                    //is_force_positive = TRUE;
                    break;
                case '#':
                    break;
                case ' ':
                    is_zero_pad = FALSE;
                case '0':
                    is_zero_pad = TRUE;
                    break;
                case '\0':
                    goto end;
                default:
                    fmt_sub_specifiers.flags = 0;
            }

            if (fmt_sub_specifiers.flags)
            {
                ch=*(fmt++);
            }

            /* Width */
            if (ch == '\0')
            {
                goto end;
            }
            if (ch == '*')    //not supported yet
            {
                ch=*(fmt++);
            }
            else if (ch >= '1' && ch <= '9')
            {
                while(ch >= '0' && ch <= '9')
                {
                    fmt_sub_specifiers.width = 1;
                    width = width * 10 + (ch - '0');
                    ch=*(fmt++);
                }
            }


            /* Precision */
            if (ch == '\0')
            {
                goto end;
            }
            if (ch == '.')
            {
                ch=*(fmt++);

                if (ch == '*')    //not supported yet
                {
                    ch=*(fmt++);
                }
                else if (ch >= '1' && ch <= '9')
                {
                    while(ch >= '0' && ch <= '9')
                    {
                        fmt_sub_specifiers.precision = 1;
                        precision = precision * 10 + (ch - '0');
                        ch=*(fmt++);
                    }
                }        
                else
                {
                    goto end;
                }        
            }

            /* length */
            if (ch == '\0')
                goto end;  
            switch(ch)
            {
              case 'l':
                    ch=*(fmt++);
                    fmt_sub_specifiers.length = 1;
                    break;
            }

            switch (ch) 
            {
                case 0:
                    goto end;

                case 'u':
                case 'd':
                    len = mini_itoa(va_arg(arglist, unsigned int), 10, 0, (ch=='u'), bf, width);
                    drv_usart_puts(DU_USART1, bf, len);
                    break;

                case 'x':
                case 'X':
                    len = mini_itoa(va_arg(arglist, unsigned int), 16, (ch=='X'), 1, bf, width);
                    drv_usart_puts(DU_USART1, bf, len);
                    break;

                case 'c' :
                    drv_usart_putc(DU_USART1, (char)(va_arg(arglist, int)));
                    break;

                case 's' :
                    ptr = va_arg(arglist, uint8_t *);
                    drv_usart_puts(DU_USART1, ptr, mini_strlen(ptr));
                    break;

                case 'f' :
                case 'F' :
                    len = mini_ftoa(va_arg(arglist, double), (ch=='F'), bf, width, precision, is_zero_pad);
                    drv_usart_puts(DU_USART1, bf, len);
                    break;

                case 'e' :
                case 'E' :
                    len = mini_ftoa(va_arg(arglist, double), (ch=='E'), bf, width, precision, is_zero_pad);
                    drv_usart_puts(DU_USART1, bf, len);
                    break;

                default:
                    drv_usart_putc(DU_USART1, ch);
                    break;
            }
        }
    }

end:
    va_end( arglist );  
    return 0;
}


//TODO: Need to check this function
int32_t mini_vsnprintf(uint8_t *buffer, uint32_t buffer_len, const uint8_t *fmt, va_list va)
{
    struct mini_buff b;
    uint8_t bf[24];
    uint8_t ch;

    b.buffer = buffer;
    b.pbuffer = buffer;
    b.buffer_len = buffer_len;

    while ((ch=*(fmt++))) {
        if ((uint32_t)((b.pbuffer - b.buffer) + 1) >= b.buffer_len)
            break;
        if (ch!='%')
        {
            mini_putc(buffer++,ch);
            b.buffer--;
        }
        else 
        {
            BOOL is_zero_pad = FALSE;
            //volatile BOOL is_force_positive = FALSE;
            uint32_t width = 0;
            uint32_t precision = 0;
            uint8_t *ptr;
            uint32_t len;

            struct
            {
                uint8_t flags : 1;
                uint8_t width : 1;
                uint8_t precision : 1;
                uint8_t length : 1;
            } fmt_sub_specifiers = {0};

            ch=*(fmt++);

            /* Flags */
            fmt_sub_specifiers.flags = 1;
            switch(ch)
            {
                case '-':
                    break;
                case '+':
                    //is_force_positive = TRUE;
                    break;
                case '#':
                    break;
                case ' ':
                    is_zero_pad = FALSE;
                case '0':
                    is_zero_pad = TRUE;
                    break;
                case '\0':
                    goto end;
                default:
                    fmt_sub_specifiers.flags = 0;
            }

            if (fmt_sub_specifiers.flags)
            {
                ch=*(fmt++);
            }

            /* Width */
            if (ch == '\0')
                goto end;
            if (ch == '*')    //not supported yet
            {
                ch=*(fmt++);
            }
            else if (ch >= '0' && ch <= '9')
            {
                while(ch >= '0' && ch <= '9')
                {
                    fmt_sub_specifiers.width = 1;
                    width = width * 10 + (ch - '0');
                    ch=*(fmt++);
                }
            }


            /* Precision */
            if (ch == '\0')
                goto end;
            if (ch == '.')
            {
                ch=*(fmt++);

                if (ch == '*')    //not supported yet
                {
                    ch=*(fmt++);
                }
                else if (ch >= '0' && ch <= '9')
                {
                    while(ch >= '0' && ch <= '9')
                    {
                        fmt_sub_specifiers.precision = 1;
                        precision = precision * 10 + (ch - '0');
                        ch=*(fmt++);
                    }
                }        
                else
                {
                    goto end;
                }        
            }

            /* length */
            if (ch == '\0')
                goto end;  
            switch(ch)
            {
              case 'l':
                    ch=*(fmt++);
                    fmt_sub_specifiers.length = 1;
                    break;
            }
            
            switch (ch) {
                case 0:
                    goto end;

                case 'u':
                case 'd':
                    len = mini_itoa(va_arg(va, unsigned int), 10, 0, (ch=='u'), bf, width);
                    mini_puts(bf, len, buffer);
                    buffer += len;
                    break;

                case 'x':
                case 'X':
                    len = mini_itoa(va_arg(va, unsigned int), 16, (ch=='X'), 1, bf, width);
                    mini_puts(bf, len, buffer);
                    buffer += len;
                    break;

                case 'c' :
                    mini_putc(buffer++,(char)(va_arg(va, int)));
                    break;

                case 's' :
                    ptr = va_arg(va, uint8_t *);
                    mini_puts(ptr, mini_strlen(ptr), buffer);
                    buffer += mini_strlen(ptr);
                    break;

                case 'f' :
                case 'F' :
                    len = mini_ftoa(va_arg(va, double), (ch=='F'), bf, width, precision, is_zero_pad);
                    mini_puts(bf, len, buffer);
                    buffer += len;
                    break;

                case 'e' :
                case 'E' :
                    len = mini_ftoa(va_arg(va, double), (ch=='E'), bf, width, precision, is_zero_pad);
                    mini_puts(bf, len, buffer);
                    buffer += len;
                    break;

                default:
                    mini_putc(buffer++,ch);
                    break;
            }
        }
    }
end:
    return b.pbuffer - b.buffer;
}


//int32_t
//mini_snprintf(uint8_t* buffer, uint32_t buffer_len, const uint8_t *fmt, ...)
//{
//    int32_t ret;
//    va_list va;
//    va_start(va, fmt);
//    ret = mini_vsnprintf(buffer, buffer_len, fmt, va);
//    va_end(va);
//
//    return ret;
//}
