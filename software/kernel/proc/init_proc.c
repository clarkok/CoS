#include <string.h>
#include <stdlib.h>

#include "init_proc.h" 

#include "lib/sysapi.h"
#include "lib/kernelio.h"

#include "driver/disk.h"

LinkedList init_proc_queue;

int
init_proc(void)
{
    k_printf(
            "Hello to init proc!\n"
            "    current pid is: 0x%X\n",
            k_get_pid()
        );

    if (!k_fork()) {
        k_set_pname("sys_idle");
        k_request_lowest();
        for (;;) ;
    }

    /*
    while (list_size(&init_proc_queue)) {
        InitProcRequest *req = list_get(list_unlink(list_head(&init_proc_queue)), InitProcRequest, _linked);
        if (!k_fork()) {
            k_set_pname(req->name);
            kernel_thread entry = req->entry;
            kfree(req);
            k_exit(entry());
        }
    }
    */

    while (true) {
        size_t sender = 0;
        size_t msg_size = k_msg_wait_for(0);
        char *buffer = k_mmap_empty(msg_size, 0);
        k_msg_recv_for(0, &sender, buffer);
        k_munmap(buffer);
    }
}
