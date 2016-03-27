#include "cos.h"
#include "kernel.h"

#include "mm/mm.h"

void __assert_fail();
void interrupt_init();

void
init()
{
    // interrupt_init();
    mm_init();

    int *p = malloc(sizeof(int));
    dbg_uart_hex((uint32_t)p);
    *p = 0;
    free(p);

    p = malloc(sizeof(int));
    dbg_uart_hex((uint32_t)p);
    *p = 1;

    p = malloc(sizeof(int));
    dbg_uart_hex((uint32_t)p);
    *p = 2;

    __assert_fail();
}
