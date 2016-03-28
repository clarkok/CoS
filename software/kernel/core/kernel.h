#ifndef _COS_KERNEL_H_
#define _COS_KERNEL_H_

#include <stddef.h>

#ifndef __TEST__

#ifdef assert
#undef assert
#endif

void kernel_panic();

#define __assert_failed(file, line, cond)               \
    do {                                                \
        dbg_uart_str("\nAssert Failed!\n" cond "\n");   \
        dbg_uart_str(file ":0x");                       \
        dbg_uart_hex(line);                             \
        kernel_panic();                                 \
    } while (0)

#define assert(cond)                        \
    do { if (!(cond)) __assert_failed(__FILE__, __LINE__, #cond); } while (0)

#else
#include <stdio.h>
#endif  // __TEST__

static inline void
out_ptb(size_t ptb)
{
#ifndef __TEST__
    __asm__ volatile (
            "sync\n"
             "\tmtc0 %0, 5"
             : : "r" (ptb)
        );
#endif
}

static inline size_t
in_ptb()
{
    size_t ptb;
#ifndef __TEST__
    __asm__ volatile (
            "mfc0 %0, 5"
            : "=r"(ptb):
        );
#else
    ptb = 0;
#endif
    return ptb;
}

static inline void
out_uart(char c)
{
#ifndef __TEST__
    __asm__ volatile (
            "sw   %0, 0xFE0C($zero)"
            : : "r" (c)
        );
#else
    printf("%c", c);
#endif
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

void interrupt_reenter();

#endif
