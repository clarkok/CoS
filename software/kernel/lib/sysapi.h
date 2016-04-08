#ifndef _COS_SYSAPI_H_
#define _COS_SYSAPI_H_

#include <stddef.h>
#include <printf.h>

/**
 * ! NOTE !
 *
 * This file MUST be synced with core/syscall_table.asm
 */

static inline void
k_dbg_uart_str(const char *str)
{
    __asm__ volatile (
            "move $a1, %0\n\t"
            "addiu $a0, $zero, 0\n\t"
            "syscall"
            : : "r"(str)
        );
}

static inline void
k_dbg_uart_hex(size_t hex)
{
    __asm__ volatile (
            "move $a1, %0\n\t"
            "addiu $a0, $zero, 1\n\t"
            "syscall"
            : : "r"(hex)
        );
}

static inline size_t
k_get_pid()
{
    size_t ret;
    __asm__ volatile (
            "addiu $a0, $zero, 2\n\t"
            "syscall\n\t"
            "move %0, $v0"
            : "=r"(ret) :
        );
    return ret;
}

static inline size_t
k_fork()
{
    size_t ret;
    __asm__ volatile (
            "addiu $a0, $zero, 3\n\t"
            "syscall\n\t"
            "move %0, $v0"
            : "=r"(ret) :
        );
    return ret;
}

static inline void
k_set_pname(const char *name)
{
    __asm__ volatile (
            "move $a1, %0\n\t"
            "addiu $a0, $zero, 4\n\t"
            "syscall"
            : : "r"(name)
        );
}

static inline void *
k_mmap_empty(size_t size, size_t hint)
{
    void *ret;

    __asm__ volatile (
            "move $a2, %1\n\t"
            "move $a1, %2\n\t"
            "addiu $a0, $zero, 5\n\t"
            "syscall\n\t"
            "move %0, $v0"
            : "=r"(ret) : "r"(hint), "r"(size)
        );

    return ret;
}

static inline void
k_munmap(void *ptr)
{
    __asm__ volatile (
            "move $a1, %0\n\t"
            "addiu $a0, $zero, 6\n\t"
            "syscall\n\t"
            : : "r"(ptr)
        );
}

static inline size_t
k_get_free_page_nr()
{
    size_t ret;

    __asm__ volatile (
            "addiu $a0, $zero, 7\n\t"
            "syscall\n\t"
            "move %0, $v0"
            : "=r"(ret) :
        );

    return ret;
}

static inline size_t
k_get_proc_nr()
{
    size_t ret;

    __asm__ volatile (
            "addiu $a0, $zero, 8\n\t"
            "syscall\n\t"
            "move %0, $v0"
            : "=r"(ret) :
        );

    return ret;
}

#endif
