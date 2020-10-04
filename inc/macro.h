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
#include "config.h"
#include "..\src\utils\mini-printf.h"

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

#define MAX(a,b)    ((a)>(b) ? (a) : (b))
#define MIN(a,b)    ((a)<(b) ? (a) : (b))

#define UPPER32(x)  ((uint32_t)((x)>>32))
#define LOWER32(x)  ((uint32_t)(x))

#define ARRAY_SIZE(x)   (sizeof(x)/sizeof(x[0]))

#define KB(x)   ((x)<<10)
#define MB(x)   ((x)<<20)

#define BIT_MASK(x)     ((1 << (x)) - 1)

#define PRINT(fmt, ...)     do { printf((uint8_t*)(fmt), ##__VA_ARGS__); } while (0)

#if defined VERBOSE || NDEBUG
    #define DEBUG_PRINT(fmt, ...)       
#elif defined DEBUG
    #define DEBUG_PRINT(fmt, ...)   PRINT(fmt"\r\n", ##__VA_ARGS__)
#endif

#define ASSERT_PRINT(x) DEBUG_PRINT("[%s](%d): "x, __FILE__, __LINE__)
#define ASSERT(message, assertion) do { if (!(assertion)) { \
            ASSERT_PRINT(message); while(1);}} while(0)

#endif  //_MACRO_