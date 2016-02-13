#include "debug.h"

#define UART_TX (0xFFFFFE0C)

void
dbg_uart_hex(uint32_t num)
{
    for (int i = 0; i < 8; i++) {
        int t = (num & 0xF0000000) >> 28;
        *(char*)(UART_TX) = (t >= 10) ? 'A' + (t - 10) : '0' + t;
        num <<= 4;
    }
}

void
dbg_uart_str(const char *str)
{
    while (*str) {
        *(char*)(UART_TX) = *str++;
    }
}
