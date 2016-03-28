    .text
interrupt_init:
    lui     $k0,    %hi(interrupt_entry)
    addiu   $k0,    $k0,    %lo(interrupt_entry)
    mtc0    $k0,    4
    jr      $ra

interrupt_entry:
    ## saving
    lui     $k0,    %hi(proc_current_scene)
    lw      $k0,    %lo(proc_current_scene)($k0)    ## load proc_current_scene

    mfc0    $k1,    0                               ## get epc
    sw      $1,     4($k0)
    sw      $2,     8($k0)
    sw      $3,     12($k0)
    sw      $4,     16($k0)
    sw      $5,     20($k0)
    sw      $6,     24($k0)
    sw      $7,     28($k0)
    sw      $8,     32($k0)
    sw      $9,     36($k0)
    sw      $10,    40($k0)
    sw      $11,    44($k0)
    sw      $12,    48($k0)
    sw      $13,    52($k0)
    sw      $14,    56($k0)
    sw      $15,    60($k0)
    sw      $16,    64($k0)
    sw      $17,    68($k0)
    sw      $18,    72($k0)
    sw      $19,    76($k0)
    sw      $20,    80($k0)
    sw      $21,    84($k0)
    sw      $22,    88($k0)
    sw      $23,    92($k0)
    sw      $24,    96($k0)
    sw      $25,    100($k0)
    sw      $28,    112($k0)
    sw      $29,    116($k0)
    sw      $30,    120($k0)
    sw      $31,    124($k0)
    sw      $k1,    0($k0)
    mflo    $k1
    sw      $k1,    128($k0)
    mfhi    $k1
    sw      $k1,    132($k0)

    lui     $k0,    0x8000
    and     $k1,    $k0,    $sp
    bnez    $k1,    _interrupt_not_set_stack
    
    ## set $sp to kernel_stack
    lui     $k0,    %hi(proc_current_scene)
    lw      $k0,    %lo(proc_current_scene)($k0)
    addiu   $sp,    $k0,    4232

_interrupt_not_set_stack:
    mfc0    $k1,    1                               ## get ecause
    lui     $k0,    %hi(INTERRUPT_HANDLER_TABLE)
    andi    $k1,    $k1,    15
    sll     $k1,    $k1,    2
    addu    $k0,    $k0,    $k1
    lw      $k1,    %lo(INTERRUPT_HANDLER_TABLE)($k0)

    jalr    $k1

interrupt_reenter:
    lui     $k0,    %hi(proc_current_scene)
    lw      $k0,    %lo(proc_current_scene)($k0)    ## load proc_current_scene again

    lw      $k1,    0($k0)
    lw      $1,     4($k0)
    lw      $2,     8($k0)
    lw      $3,     12($k0)
    lw      $4,     16($k0)
    lw      $5,     20($k0)
    lw      $6,     24($k0)
    lw      $7,     28($k0)
    lw      $8,     32($k0)
    lw      $9,     36($k0)
    lw      $10,    40($k0)
    lw      $11,    44($k0)
    lw      $12,    48($k0)
    lw      $13,    52($k0)
    lw      $14,    56($k0)
    lw      $15,    60($k0)
    lw      $16,    64($k0)
    lw      $17,    68($k0)
    lw      $18,    72($k0)
    lw      $19,    76($k0)
    lw      $20,    80($k0)
    lw      $21,    84($k0)
    lw      $22,    88($k0)
    lw      $23,    92($k0)
    lw      $24,    96($k0)
    lw      $25,    100($k0)
    lw      $28,    112($k0)
    lw      $29,    116($k0)
    lw      $30,    120($k0)
    lw      $31,    124($k0)
    mtc0    $k1,    0                               ## restore epc
    lw      $k1,    128($k0)
    mtlo    $k1
    lw      $k1,    132($k0)
    mthi    $k1

    eret

interrupt_default_handler:
    jr      $ra                                     ## do nothing

interrupt_hardware_reset_handler:
    addiu   $k0,    $zero,  0xF000
    jr      $k0

interrupt_unexpected_pagefault_handler:
    lui     $a0,    %hi(INTERRUPT_UNEXPECTED_PAGE_FAULT)
    addiu   $a0,    $a0,    %lo(INTERRUPT_UNEXPECTED_PAGE_FAULT)
    jal     interrupt_uart_str

    mfc0    $a0,    0
    jal     interrupt_uart_hex

    lui     $a0,    %hi(INTERRUPT_UNEXPECTED_PAGE_FAULT_PFA)
    addiu   $a0,    $a0,    %lo(INTERRUPT_UNEXPECTED_PAGE_FAULT_PFA)
    jal     interrupt_uart_str

    mfc0    $a0,    6
    jal     interrupt_uart_hex

    lui     $a0,    %hi(INTERRUPT_RETURN_LINE)
    addiu   $a0,    $a0,    %lo(INTERRUPT_RETURN_LINE)
    jal     interrupt_uart_str

    j       kernel_panic

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
    .4byte  (interrupt_unexpected_pagefault_handler)            ## pagefault
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
    .4byte  (interrupt_unexpected_exception_handler)            ## syscall
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
