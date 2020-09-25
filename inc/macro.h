/**
 * @file macro.h
 * @author Alexey Nikolaev (alexeynikzzz@gmail.com)
 * @brief Here're placed common macros
 * @version 0.1
 * @date 2020-05-30
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#ifndef _MACRO_
#define _MACRO_

#include <stdint.h>

typedef uint8_t         BOOL8;
typedef uint16_t        BOOL16;
typedef uint32_t        BOOL32;
typedef uint32_t        BOOL;

#define FALSE           ((BOOL) 0)
#define TRUE            ((BOOL) 1)

#define NOP()           do{ __asm volatile ("nop");}while(0)

#define INLINE          __inline
#define FORCE_INLINE    inline __attribute__(( always_inline))

#define KHZ             1000
#define MHZ             (1000 * KHZ)

#define BIT_MASK(x)     ((1 << (x)) - 1)

#endif  //_MACRO_