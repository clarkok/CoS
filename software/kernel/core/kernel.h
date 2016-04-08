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

enum InterruptType
{
    INT_HARDWARE_RESET  = 0,
    INT_PAGEFAULT       = 1,
    INT_VGA             = 2,
    INT_FLASH           = 3,
    INT_UART            = 4,
    INT_TIMER           = 5,
    INT_PS2             = 6,

    EXC_UNALIGNED_INST  = 8,
    EXC_UNALIGNED_DATA  = 9,
    EXC_UNDEFINED_INST  = 10,
    EXC_PRIVILEGE_INST  = 11,
    EXC_PRIVILEGE_ADDR  = 12,
    EXC_OVERFLOW        = 13,
    EXC_SYSCALL         = 14,
    EXC_BREAK           = 15
};

extern size_t INTERRUPT_HANDLER_TABLE[];

#ifndef __TEST__

#define mfc0(dst, id)                   \
    __asm__ volatile (                  \
            "mfc0 %0, " #id "\n\t"      \
            "nop"                       \
            : "=r"(dst) :               \
        )

#define mtc0(src, id)                   \
    __asm__ volatile (                  \
            "mtc0 %0, " #id "\n\t"      \
            "nop"                       \
            : : "r"(src)                \
        )

static inline void
out_epc(size_t epc)
{ mtc0(epc, 0); }

static inline size_t
in_epc()
{
    size_t epc;
    mfc0(epc, 0);
    return epc;
}

static inline size_t
in_ecause()
{
    size_t ecause;
    mfc0(ecause, 1);
    return ecause;
}

static inline void
out_ie(size_t ie)
{ mtc0(ie, 2); }

static inline size_t
in_ie()
{
    size_t ie;
    mfc0(ie, 2);
    return ie;
}

static inline size_t
in_is()
{
    size_t is;
    mfc0(is, 3);
    return is;
}

static inline void
out_ptb(size_t ptb)
{
    __asm__ volatile ("sync");
    mtc0(ptb, 5);
}

static inline size_t
in_ptb()
{
    size_t ptb;
    mfc0(ptb, 5);
    return ptb;
}

static inline size_t
in_pfa()
{
    size_t pfa;
    mfc0(pfa, 6);
    return pfa;
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
enable_global_int()
{ out_ie(in_ie() | 0x80000000); }

static inline void
disable_global_int()
{ out_ie(in_ie() & 0x7FFFFFFF); }

static inline void
enable_int(int num)
{ out_ie(in_ie() | (1 << num)); }

static inline void
disable_int(int num)
{ out_ie(in_ie() & ~(1 << num)); }

#else   // __TEST__

static inline void
out_ptb(size_t ptb)
{ printf("out ptb: %u\n", ptb); }

static inline size_t
in_ptb()
{ return 0; }

static inline void
out_uart(char c)
{ printf("%c", c); }

#endif  // __TEST__

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

int kprintf(const char *format, ...);

void process_enter();

#define log(func, in)   \
    kprintf("%s " in ", proc: 0x%x, kstack: 0x%x, ksttop: 0x%x\n", func,    \
            current_process->id, current_process->kernel_stack, current_process->kernel_stack_top)

#define log_in(func)    \
    log(#func, "<")

#define log_out(func)   \
    log(#func, ">")

#endif
