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

static inline int
k_msg_send(size_t dst, size_t length, const void *content)
{
    int ret;

    __asm__ volatile (
            "move $a3, %3\n\t"
            "move $a2, %2\n\t"
            "move $a1, %1\n\t"
            "addiu $a0, $zero, 9\n\t"
            "syscall\n\t"
            "move %0, $v0"
            : "=r"(ret) : "r"(dst), "r"(length), "r"(content)
        );

    return ret;
}

static inline size_t
k_msg_wait_for(size_t src)
{
    size_t ret;

    __asm__ volatile (
            "move $a1, %1\n\t"
            "addiu $a0, $zero, 10\n\t"
            "syscall\n\t"
            "move %0, $v0"
            : "=r"(ret) : "r"(src)
        );

    return ret;
}

static inline int
k_msg_recv_for(size_t src, size_t *actual_src, char *buffer)
{
    int ret;

    __asm__ volatile (
            "move $a3, %3\n\t"
            "move $a2, %2\n\t"
            "move $a1, %1\n\t"
            "addiu $a0, $zero, 11\n\t"
            "syscall\n\t"
            "move %0, $v0"
            : "=r"(ret) : "r"(src), "r"(actual_src), "r"(buffer)
        );

    return ret;
}

static inline size_t
k_get_msg_nr()
{
    int ret;

    __asm__ volatile (
            "addiu $a0, $zero, 12\n\t"
            "syscall\n\t"
            "move %0, $v0"
            : "=r"(ret) :
        );

    return ret;
}

static inline void
k_exit(int retval)
{
    __asm__ volatile (
            "move $a1, %0\n\t"
            "addiu $a0, $zero, 13\n\t"
            "syscall"
            : : "r"(retval)
        );

    assert(0);
}

static inline int
k_collect(size_t child_pid, int *retval)
{
    int ret;

    __asm__ volatile (
            "move $a2, %2\n\t"
            "move $a1, %1\n\t"
            "addiu $a0, $zero, 14"
            "syscall\n\t"
            "move %0, $v0"
            : "=r"(ret) : "r"(child_pid), "r"(retval)
        );

    return ret;
}

static inline void
k_giveup()
{
    __asm__ volatile (
            "addiu $a0, $zero, 15"
            "syscall\n\t"
        );
}

static inline void
k_request_lowest()
{
    __asm__ volatile (
            "addiu $a0, $zero, 16"
            "syscall\n\t"
        );
}

#endif
