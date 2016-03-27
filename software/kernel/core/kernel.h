#ifndef _COS_KERNEL_H_
#define _COS_KERNEL_H_

#include <stddef.h>

static inline void
out_ptb(size_t ptb)
{
    __asm__ ("sync\n"
             "\tmtc0 %0, 5"
             : : "r" (ptb)
        );
}

static inline void
out_uart(char c)
{
    __asm__ volatile (
            "sw   %0, 0xFE0C($zero)"
            : : "r" (c)
        );
}

static inline void
dbg_uart_str(const char *str)
{ while (*str) { out_uart(*str++); } }

static inline void
dbg_uart_hex(uint32_t x)
{
    static const char XDIGIT_TABLE[] = "0123456789ABCDEF";

    char buffer[9];

    for (int i = 7; i >= 0; i--) {
        uint32_t xdigit = x & 0xF;
        buffer[i] = XDIGIT_TABLE[xdigit];
        x >>= 4;
    }

    buffer[8] = 0;
    dbg_uart_str(buffer);
    dbg_uart_str("\n");
}

#endif
