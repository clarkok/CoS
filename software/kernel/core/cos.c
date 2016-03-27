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

    __assert_fail();
}
