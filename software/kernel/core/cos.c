#include "cos.h"
#include "kernel.h"

#include "mm/mm.h"

void __assert_fail();

void
init()
{
    dbg_uart_str("Kernel initializing\n");
    mm_init();
    dbg_uart_str("Kernel MM inited\n");

    __assert_fail();
}
