#ifndef _COS_UTILS_H_
#define _COS_UTILS_H_

#include <stdint.h>

#include "core/kernel.h"

#define array_item_count(arr)   \
    (sizeof(arr) / sizeof(arr[0]))

static uint32_t
minu32(uint32_t a, uint32_t b) __ALWAYS_INLINE__
{ return (a > b) ? b : a; }

static uint32_t
maxu32(uint32_t a, uint32_t b) __ALWAYS_INLINE__
{ return (a > b) ? a : b; }

static uint16_t
minu16(uint16_t a, uint16_t b) __ALWAYS_INLINE__
{ return (a > b) ? b : a; }

static uint16_t
maxu16(uint16_t a, uint16_t b) __ALWAYS_INLINE__
{ return (a > b) ? a : b; }

static uint8_t
minu8(uint8_t a, uint8_t b) __ALWAYS_INLINE__
{ return (a > b) ? b : a; }

static uint8_t
maxu8(uint8_t a, uint8_t b) __ALWAYS_INLINE__
{ return (a > b) ? a : b; }

inline uint32_t
get_lsb_idx(uint32_t a) __WEAK__
{
    if      (a & 0x00000001)    return 0;
    else if (a & 0x00000002)    return 1;
    else if (a & 0x00000004)    return 2;
    else if (a & 0x00000008)    return 3;
    else if (a & 0x00000010)    return 4;
    else if (a & 0x00000020)    return 5;
    else if (a & 0x00000040)    return 6;
    else if (a & 0x00000080)    return 7;
    else if (a & 0x00000100)    return 8;
    else if (a & 0x00000200)    return 9;
    else if (a & 0x00000400)    return 10;
    else if (a & 0x00000800)    return 11;
    else if (a & 0x00001000)    return 12;
    else if (a & 0x00002000)    return 13;
    else if (a & 0x00004000)    return 14;
    else if (a & 0x00008000)    return 15;
    else if (a & 0x00010000)    return 16;
    else if (a & 0x00020000)    return 17;
    else if (a & 0x00040000)    return 18;
    else if (a & 0x00080000)    return 19;
    else if (a & 0x00100000)    return 20;
    else if (a & 0x00200000)    return 21;
    else if (a & 0x00400000)    return 22;
    else if (a & 0x00800000)    return 23;
    else if (a & 0x01000000)    return 24;
    else if (a & 0x02000000)    return 25;
    else if (a & 0x04000000)    return 26;
    else if (a & 0x08000000)    return 27;
    else if (a & 0x10000000)    return 28;
    else if (a & 0x20000000)    return 29;
    else if (a & 0x40000000)    return 30;
    else                        return 31;
}

inline uint32_t
get_msb_idx(uint32_t a) __WEAK__
{
    if      (a & 0x80000000)    return 31;
    else if (a & 0x40000000)    return 30;
    else if (a & 0x20000000)    return 29;
    else if (a & 0x10000000)    return 28;
    else if (a & 0x08000000)    return 27;
    else if (a & 0x04000000)    return 26;
    else if (a & 0x02000000)    return 25;
    else if (a & 0x01000000)    return 24;
    else if (a & 0x00800000)    return 23;
    else if (a & 0x00400000)    return 22;
    else if (a & 0x00200000)    return 21;
    else if (a & 0x00100000)    return 20;
    else if (a & 0x00080000)    return 19;
    else if (a & 0x00040000)    return 18;
    else if (a & 0x00020000)    return 17;
    else if (a & 0x00010000)    return 16;
    else if (a & 0x00008000)    return 15;
    else if (a & 0x00004000)    return 14;
    else if (a & 0x00002000)    return 13;
    else if (a & 0x00001000)    return 12;
    else if (a & 0x00000800)    return 11;
    else if (a & 0x00000400)    return 10;
    else if (a & 0x00000200)    return 9;
    else if (a & 0x00000100)    return 8;
    else if (a & 0x00000080)    return 7;
    else if (a & 0x00000040)    return 6;
    else if (a & 0x00000020)    return 5;
    else if (a & 0x00000010)    return 4;
    else if (a & 0x00000008)    return 3;
    else if (a & 0x00000004)    return 2;
    else if (a & 0x00000002)    return 1;
    else                        return 0;
}

#endif // _COS_UTILS_H_
