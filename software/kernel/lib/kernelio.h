#ifndef _COS_KERNEL_IO_H_
#define _COS_KERNEL_IO_H_

#include "sysapi.h"

static inline int
k_printf(const char *format, ...)
{
    char *buffer = (char*)k_mmap_empty(4096, -1);
    int ret;
    va_list vl;
    va_start(vl, format);
    ret = vsprintf(buffer, format, vl);
    k_dbg_uart_str(buffer);
    va_end(vl);
    k_munmap(buffer);
    return ret;
}

#endif
