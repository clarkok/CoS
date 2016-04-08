#include <printf.h>

#include "lib/sysapi.h"
#include "lib/kernelio.h"

void
init_proc()
{
    k_printf(
            "Hello to init proc!\n"
            "    current pid is: 0x%X\n",
            k_get_pid()
        );

    for (int i = 0; i < 5; ++i) {
        k_fork();
    }

    for (;;) {
    }
}
