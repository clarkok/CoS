#include "cos.h"
#include "kernel.h"

#include "mm/mm.h"
#include "proc/proc.h"

void __assert_fail();
void interrupt_init();

void
init()
{
    mm_init();
    proc_init();
    interrupt_init();

    interrupt_reenter();

    __assert_fail();
}
