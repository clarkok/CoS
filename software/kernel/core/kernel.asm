    .text
_krnl_init:
    addiu   $a0,    $zero,  1
    lui     $t0,    0x4000
    lui     $a1,    %hi(_KERNEL_RUNNING)
    addu    $a1,    $a1,    $t0
    addiu   $a1,    $a1,    %lo(_KERNEL_RUNNING)
    syscall

    ## memset(0xC0000000, 0, 0x00100000)
    ## initialize lower 1MB to zero
    addiu   $a0,    $zero,  11
    lui     $a1,    0xC000
    move    $a2,    $zero
    lui     $a3,    0x0010
    syscall

    addiu   $a0,    $zero,  1
    lui     $t0,    0x4000
    lui     $a1,    %hi(_KERNEL_MEMSET)
    addu    $a1,    $a1,    $t0
    addiu   $a1,    $a1,    %lo(_KERNEL_MEMSET)
    syscall

    ## initialize first 8MB page table, and stack space
    lui     $t1,    0xC020          ## page_dir

    addiu   $t0,    $zero,  0x0001
    sw      $t0,    2048($t1)
    addiu   $t0,    $zero,  0x1001
    sw      $t0,    2052($t1)
    lui     $t0,    %hi(0x000FF001)
    addiu   $t0,    $t0,    %lo(0x000FF001)
    sw      $t0,    3068($t1)

    lui     $t1,    0xC000
    addiu   $t0,    $zero,  0x0003
    addiu   $t2,    $zero,  2048

    b       _krnl_fill_kernel_page_table_cmp
_krnl_fill_kernel_page_table_loop:
    sw      $t0,    0($t1)
    addiu   $t2,    $t2,    -1
    addiu   $t0,    $t0,    4096
    addiu   $t1,    $t1,    4
_krnl_fill_kernel_page_table_cmp:
    bnez    $t2,    _krnl_fill_kernel_page_table_loop

    ## initialize stack space
    lui     $t0,    0x0020
    lui     $t1,    0xC010
    addiu   $t0,    $t0,    -4093
    sw      $t0,    -4($t1)

    lui     $t0,    0x0020
    sync
    mtc0    $t0,    5

    lui     $sp,    0xC000

    lui     $t0,    %hi(init)
    addiu   $t0,    $t0,    %lo(init)
    jalr    $t0

__assert_fail:
    break

kernel_panic:
    addiu   $k0,    $zero,  0xFE0C
    lui     $k1,    %hi(_KERNEL_PANIC)
    addiu   $k1,    $k1,    %lo(_KERNEL_PANIC)
_kernel_panic_output_loop:
    lbu     $t0,    0($k1)
    sw      $t0,    0($k0)
    addiu   $k1,    $k1,    1
_kernel_panic_output_cmp:
    bnez    $t0,    _kernel_panic_output_loop
    j       kernel_panic

    .section ".rodata"
_KERNEL_RUNNING:
    .asciiz "kernel running!\n"
_KERNEL_MEMSET:
    .asciiz "kernel memset!\n"
_KERNEL_PANIC:
    .asciiz "kernel panic!\n"
