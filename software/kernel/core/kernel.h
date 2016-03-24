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

#endif
