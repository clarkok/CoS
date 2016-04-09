#include <string.h>

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

    if (!k_fork()) {
        k_set_pname("sys_idle");
        for (;;) ;
    }

    int child_pid = k_fork();

    if (child_pid) {
        while (1) {
            size_t msg_length = k_msg_wait_for(0);
            char *msg_content = k_mmap_empty(msg_length, -1);
            size_t src;

            k_msg_recv_for(0, &src, msg_content);

            if (src == 0xFFFFFFFFu) {
                if (((size_t*)(msg_content))[0] == 2) {
                    k_printf("Children 0x%x turned to zombie, I must destroy it\n", 
                            ((size_t*)(msg_content))[1]
                        );
                    int retval,
                        result;

                    result = k_collect(((size_t*)(msg_content))[1], &retval);
                    assert(result);

                    k_printf("Children 0x%x last word is 0x%x\n", ((size_t*)(msg_content))[1], retval);
                }
            }

            k_munmap(msg_content);
        }
    }
    else {
        k_printf("I am the lonely children. I would die after a was born\n");

        if (k_fork()) {
            k_printf("Now I will go die\n");
            k_exit(8);
        }
        else {
            k_exit(9);
        }
    }

    for (;;) {
    }
}
