#include "proc/init_proc.h"
#include "lib/kernelio.h"

#include "fs.h"

static int fs_proc(void);

void
fs_init()
{
    kprintf("FS init\n");
    proc_request_init("filesystem", fs_proc);
}

size_t disk_pid = 0;

static int
fs_proc(void)
{
    while (!disk_pid) {
        disk_pid = init_proc_lookup_name("disk");
    }

    k_printf("fs proc running, disk_pid is 0x%x\n", disk_pid);

    while (1) {};
    return 0;
}
