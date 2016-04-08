SYSCALL_TABLE:
    .4byte  (dbg_uart_str)              # 0
    .4byte  (dbg_uart_hex)              # 1
    .4byte  (proc_do_get_pid)           # 2
    .4byte  (proc_do_fork)              # 3
    .4byte  (proc_do_set_pname)         # 4
    .4byte  (mm_do_mmap_empty)          # 5
    .4byte  (mm_do_munmap)              # 6
    .4byte  (mm_do_get_free_page_nr)    # 7
