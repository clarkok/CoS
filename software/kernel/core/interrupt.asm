    .text
interrupt_init:
    lui     $k0,    %hi(interrupt_entry)
    addiu   $k0,    $k0,    %lo(interrupt_entry)
    mtc0    $k0,    4
    jr      $ra

process_enter:
    lui     $k0,    %hi(current_process)
    lw      $k0,    %lo(current_process)($k0)
    lw      $sp,    0($k0)      # load kernel_stack_top
    j       interrupt_leave

interrupt_entry:
    lui     $k0,    %hi(current_process)
    lw      $k0,    %lo(current_process)($k0)
    lw      $k1,    0($k0)      # load kernel_stack_top
    addiu   $k1,    $k1,    -140
    sw      $k1,    0($k0)
    lw      $k0,    8($k0)
    sw      $k0,    136($k1)

    ## update current_scene
    lui     $k0,    %hi(current_process)
    lw      $k0,    %lo(current_process)($k0)
    sw      $k1,    8($k0)

    mfc0    $k0,    0
    sw      $1,     4($k1)
    sw      $2,     8($k1)
    sw      $3,     12($k1)
    sw      $4,     16($k1)
    sw      $5,     20($k1)
    sw      $6,     24($k1)
    sw      $7,     28($k1)
    sw      $8,     32($k1)
    sw      $9,     36($k1)
    sw      $10,    40($k1)
    sw      $11,    44($k1)
    sw      $12,    48($k1)
    sw      $13,    52($k1)
    sw      $14,    56($k1)
    sw      $15,    60($k1)
    sw      $16,    64($k1)
    sw      $17,    68($k1)
    sw      $18,    72($k1)
    sw      $19,    76($k1)
    sw      $20,    80($k1)
    sw      $21,    84($k1)
    sw      $22,    88($k1)
    sw      $23,    92($k1)
    sw      $24,    96($k1)
    sw      $25,    100($k1)
    sw      $28,    112($k1)
    sw      $29,    116($k1)
    sw      $30,    120($k1)
    sw      $31,    124($k1)
    sw      $k0,    0($k1)
    mflo    $k0
    sw      $k0,    128($k1)
    mfhi    $k0
    sw      $k0,    132($k1)

    move    $sp,    $k1

    mfc0    $k0,    1
    lui     $k1,    %hi(INTERRUPT_HANDLER_TABLE)
    sll     $k0,    $k0,    2
    addu    $k1,    $k1,    $k0
    lw      $k0,    %lo(INTERRUPT_HANDLER_TABLE)($k1)

    jalr    $k0

interrupt_leave:
    # disable global interrupt
    mfc0    $t0,    2
    lui     $t1,    %hi(0x7FFFFFFF)
    addiu   $t1,    $t1,    %lo(0x7FFFFFFF)
    and     $t0,    $t0,    $t1
    mtc0    $t0,    2

    lui     $k0,    %hi(current_process)
    lw      $k0,    %lo(current_process)($k0)
    lw      $k1,    0($k0)      # load kernel_stack_top
    addiu   $k1,    $k1,    140
    sw      $k1,    0($k0)
    addiu   $k1,    $k1,    -140

    lw      $t0,    136($k1)
    sw      $t0,    8($k0)

    lw      $k0,    0($k1)
    mtc0    $k0,    0
    lw      $1,     4($k1)
    lw      $2,     8($k1)
    lw      $3,     12($k1)
    lw      $4,     16($k1)
    lw      $5,     20($k1)
    lw      $6,     24($k1)
    lw      $7,     28($k1)
    lw      $8,     32($k1)
    lw      $9,     36($k1)
    lw      $10,    40($k1)
    lw      $11,    44($k1)
    lw      $12,    48($k1)
    lw      $13,    52($k1)
    lw      $14,    56($k1)
    lw      $15,    60($k1)
    lw      $16,    64($k1)
    lw      $17,    68($k1)
    lw      $18,    72($k1)
    lw      $19,    76($k1)
    lw      $20,    80($k1)
    lw      $21,    84($k1)
    lw      $22,    88($k1)
    lw      $23,    92($k1)
    lw      $24,    96($k1)
    lw      $25,    100($k1)
    lw      $28,    112($k1)
    lw      $29,    116($k1)
    lw      $30,    120($k1)
    lw      $31,    124($k1)
    lw      $k0,    128($k1)
    mtlo    $k0
    lw      $k0,    132($k1)
    mthi    $k0

    eret

syscall_handler:
    addiu   $sp,    $sp,    -4
    sw      $ra,    0($sp)

    ## enable interrupt
#   mfc0    $t0,    2
#   lui     $t1,    0x8000
#   or      $t0,    $t0,    $t1
#   mtc0    $t0,    2

    lui     $t0,    %hi(proc_request_schedule)
    sw      $zero,  %lo(proc_request_schedule)($t0)

    sll     $t1,    $a0,    2
    lui     $t0,    %hi(SYSCALL_TABLE)
    addu    $t0,    $t0,    $t1
    lw      $t0,    %lo(SYSCALL_TABLE)($t0)

    move    $a0,    $a1
    move    $a1,    $a2
    move    $a2,    $a3
    move    $a3,    $zero
    jalr    $t0

    ## disable interrupt
#   mfc0    $t0,    2
#   lui     $t1,    %hi(0x7FFFFFFF)
#   addiu   $t1,    $t1,    %lo(0x7FFFFFFF)
#   and     $t0,    $t0,    $t1
#   mtc0    $t0,    2

    lui     $t0,    %hi(current_process)
    lw      $t0,    %lo(current_process)($t0)
    lw      $t1,    0($t0)
    sw      $v0,    8($t1)

    lui     $t0,    %hi(proc_request_schedule)
    lw      $t1,    %lo(proc_request_schedule)($t0)

    beqz    $t1,    _syscall_no_need_schedule

    jal     proc_schedule

_syscall_no_need_schedule:
    lw      $ra,    0($sp)
    addiu   $sp,    $sp,    4
    jr      $ra

interrupt_default_handler:
    jr      $ra                                     ## do nothing

interrupt_hardware_reset_handler:
    addiu   $k0,    $zero,  0xF000
    jr      $k0

interrupt_unexpected_exception_handler:
    lui     $a0,    %hi(INTERRUPT_UNEXPECTED_EXCEPTION)
    addiu   $a0,    $a0,    %lo(INTERRUPT_UNEXPECTED_EXCEPTION)
    jal     interrupt_uart_str

    mfc0    $a0,    1
    jal     interrupt_uart_hex

    lui     $a0,    %hi(INTERRUPT_UNEXPECTED_EXCEPTION_EPC)
    addiu   $a0,    $a0,    %lo(INTERRUPT_UNEXPECTED_EXCEPTION_EPC)
    jal     interrupt_uart_str

    mfc0    $a0,    0
    jal     interrupt_uart_hex

    lui     $a0,    %hi(INTERRUPT_RETURN_LINE)
    addiu   $a0,    $a0,    %lo(INTERRUPT_RETURN_LINE)
    jal     interrupt_uart_str

    j       kernel_panic

# param #0 str
interrupt_uart_str:
    addiu   $a1,    $zero,  0xFE0C
_interrupt_uart_str_loop:
    lbu     $a2,    0($a0)
    beqz    $a2,    _interrupt_uart_str_loop_end
    sw      $a2,    0($a1)
    addiu   $a0,    $a0,    1
    j       _interrupt_uart_str_loop
_interrupt_uart_str_loop_end:
    jr      $ra

# param #0 num
interrupt_uart_hex:
    addiu   $k0,    $zero,  0xFE0C
    lui     $a2,    0xF000
    addiu   $a1,    $zero,  8

    j       _interrupt_uart_hex_cmp
_interrupt_uart_hex_loop:
    and     $k1,    $a0,    $a2
    sll     $a0,    $a0,    4
    srl     $k1,    $k1,    28
    slti    $at,    $k1,    0xA
    bnez    $at,    _interrupt_uart_hex_digit
    addiu   $k1,    $k1,    55
    j       _interrupt_uart_hex_out
_interrupt_uart_hex_digit:
    addiu   $k1,    $k1,    48
_interrupt_uart_hex_out:
    sw      $k1,    0($k0)
_interrupt_uart_hex_cmp:
    addiu   $a1,    $a1,    -1
    bgez    $a1,    _interrupt_uart_hex_loop

    jr      $ra

    .data
INTERRUPT_HANDLER_TABLE:
    .4byte  (interrupt_hardware_reset_handler)                  ## hardware reset
    .4byte  (mm_pagefault_handler)                              ## pagefault
    .4byte  (interrupt_default_handler)                         ## vga
    .4byte  (interrupt_default_handler)                         ## flash
    .4byte  (interrupt_default_handler)                         ## uart
    .4byte  (interrupt_default_handler)                         ## timer
    .4byte  (interrupt_default_handler)                         ## ps2
    .4byte  (interrupt_unexpected_exception_handler)            ## unused

    .4byte  (interrupt_unexpected_exception_handler)            ## unaligned inst
    .4byte  (interrupt_unexpected_exception_handler)            ## unaligned data
    .4byte  (interrupt_unexpected_exception_handler)            ## undefined inst
    .4byte  (interrupt_unexpected_exception_handler)            ## privilege inst
    .4byte  (interrupt_unexpected_exception_handler)            ## privilege addr
    .4byte  (interrupt_unexpected_exception_handler)            ## overflow
    .4byte  (syscall_handler)                                   ## syscall
    .4byte  (interrupt_unexpected_exception_handler)            ## break

    .section ".rodata"
INTERRUPT_UNEXPECTED_PAGE_FAULT:
    .asciiz "Unexpected pagefault!\n    epc: "
INTERRUPT_UNEXPECTED_PAGE_FAULT_PFA:
    .asciiz "    pfa: "
INTERRUPT_UNEXPECTED_EXCEPTION:
    .asciiz "Unexpected exception!\n    cause: "
INTERRUPT_UNEXPECTED_EXCEPTION_EPC:
    .asciiz "    epc: "
INTERRUPT_RETURN_LINE:
    .asciiz "\n"
