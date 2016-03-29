#include "cos.h"
#include "kernel.h"

#include "mm/mm.h"
#include "proc/proc.h"
#include "driver/driver.h"

void __assert_fail();
void interrupt_init();

void
init()
{
    disable_global_int();

    mm_init();
    proc_init();
    driver_init();

    interrupt_init();
    interrupt_reenter();

    __assert_fail();
}
