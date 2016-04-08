#include <printf.h>

#include "kernel.h"

int
kprintf(const char *format, ...)
{
    static char buffer[512];
    int ret;
    va_list vl;

    va_start(vl, format);
    ret = vsprintf(buffer, format, vl);
    dbg_uart_str(buffer);
    return ret;
}
