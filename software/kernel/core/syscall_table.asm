## NOTE:
##   this file should keep synced with lib/sysapi.h

SYSCALL_TABLE:
    .4byte  (dbg_uart_str)              # 0
    .4byte  (dbg_uart_hex)              # 1
    .4byte  (proc_do_get_pid)           # 2
    .4byte  (proc_do_fork)              # 3
    .4byte  (proc_do_set_pname)         # 4
    .4byte  (mm_do_mmap_empty)          # 5
    .4byte  (mm_do_munmap)              # 6
    .4byte  (mm_do_get_free_page_nr)    # 7
    .4byte  (proc_do_get_proc_nr)       # 8
    .4byte  (proc_msg_do_send)          # 9
    .4byte  (proc_msg_do_wait_for)      # 10
    .4byte  (proc_msg_do_recv_for)      # 11
    .4byte  (proc_msg_do_get_msg_nr)    # 12
    .4byte  (proc_do_exit)              # 13
    .4byte  (proc_do_collect)           # 14
    .4byte  (proc_do_giveup)            # 15
    .4byte  (proc_do_request_lowest)    # 16
