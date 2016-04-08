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

    int child_pid = k_fork();

    if (child_pid) {
        size_t msg_length = k_msg_wait_for(0);
        char *msg_content = k_mmap_empty(msg_length, -1);
        k_msg_recv_for(0, msg_content);

        k_printf("received: \"%s\"\n", msg_content);
    }
    else {
        const char msg[] = "Hello from the child";

        k_msg_send(1, strlen(msg) + 1, msg);

        k_printf("sent: \"%s\"\n", msg);
    }

    for (;;) {
    }
}
