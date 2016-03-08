#ifndef _COS_BITS_H_
#define _COS_BITS_H_

#include <stdint.h>

static inline int
bits_lsb_idx_8(uint8_t x)
{
    return (
        (x & 0x0F)
            ? (x & 0x03)
                ? (x & 0x01)
                    ? 0
                    : 1
                : (x & 0x04)
                    ? 2
                    : 3
            : (x & 0x30)
                ? (x & 0x10)
                    ? 4
                    : 5
                : (x & 0x40)
                    ? 6
                    : 7
    );
}

static inline int
bits_lsb_idx_16(uint16_t x)
{ return (x & 0x00FF) ? (bits_lsb_idx_8(x & 0x00FF)) : (bits_lsb_idx_8(x >> 8) + 8); }

static inline int
bits_lsb_idx_32(uint32_t x)
{ return (x & 0x0000FFFF) ? (bits_lsb_idx_16(x & 0x0000FFFF)) : (bits_lsb_idx_16(x >> 16) + 16); }

static inline int
bits_msb_idx_8(uint8_t x)
{
    return (
        (x & 0xF0)
            ? (x & 0xC0)
                ? (x & 0x80)
                    ? 7
                    : 6
                : (x & 0x20)
                    ? 5
                    : 4
            : (x & 0x0C)
                ? (x & 0x08)
                    ? 3
                    : 2
                : (x & 0x02)
                    ? 1
                    : 0
    );
}

static inline int
bits_msb_idx_16(uint16_t x)
{ return (x >> 8) ? (bits_msb_idx_8(x >> 8) + 8) : (bits_msb_idx_8(x & 0x00FF)); }

static inline int
bits_msb_idx_32(uint32_t x)
{ return (x >> 16) ? (bits_msb_idx_16(x >> 16) + 16) : (bits_msb_idx_16(x & 0x0000FFFF)); }

#endif
