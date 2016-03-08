#ifndef _COS_KERNEL_H_
#define _COS_KERNEL_H_

#include <stddef.h>

inline
void out_ptb(size_t ptb)
{
    __asm__ ("sync\n"
             "\tmtc0 %0, 5"
             : : "r" (ptb)
        );
}

inline
void out_uart(char c)
{
    __asm__ volatile (
            "sw   %0, 0xFE0C($zero)"
            : : "r" (c)
        );
}

void dbg_uart_str(const char *);
void dbg_uart_hex(size_t);

#endif
