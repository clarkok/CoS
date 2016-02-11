    .text
loader_init:
    addiu   $a0,    $zero,  1   # bios_uart_str
    lui     $a1,    %hi(LOADER_START)
    addiu   $a1,    $a1,    %lo(LOADER_START)
    syscall

    lui     $at,    0xC000
    addiu   $at,    $at,    512
    lw      $at,    0($at)  # get sector_start from boot.asm
    lui     $t0,    %hi(sector_start)
    sw      $at,    %lo(sector_start)($t0)

    # load superblock to 0xC0000000
    addiu   $a0,    $zero,  6   # bios_block_read
    move    $a1,    $at
    addiu   $a2,    $zero,  1
    lui     $a3,    0xC000
    syscall

    addiu   $a0,    $zero,  1   # bios_uart_str
    lui     $a1,    %hi(LOADER_TOTAL_INODES)
    addiu   $a1,    $a1,    %lo(LOADER_TOTAL_INODES)
    syscall

    addiu   $a0,    $zero,  2   # bios_uart_hex
    lui     $a1,    0xC000
    lhu     $a1,    56($a1)
    syscall

    addiu   $a0,    $zero,  1   # bios_uart_str
    lui     $a1,    %hi(LOADER_RETURN_LINE)
    addiu   $a1,    $a1,    %lo(LOADER_RETURN_LINE)
    syscall

    # load block group table to 0xC0000400
    addiu   $a0,    $zero,  6   # bios_block_read
    lui     $at,    %hi(sector_start)
    lw      $a1,    %lo(sector_start)($at)
    addiu   $a2,    $zero,  2
    lui     $a3,    0xC000
    addiu   $a3,    $a3,    1024
    syscall

    addiu   $a0,    $zero,  1   # bios_uart_str
    lui     $a1,    %hi(LOADER_GROUP_TABLE_LOADED)
    addiu   $a1,    $a1,    %lo(LOADER_GROUP_TABLE_LOADED)
    syscall

    # load inode table for root dir to 0xC0000800
    addiu   $a0,    $zero,  6   # bios_block_read
    lui     $at,    %hi(sector_start)
    lw      $a1,    %lo(sector_start)($at)
    lui     $at,    0xC000
    lw      $a2,    1032($at)
    addiu   $a3,    $at,    2048
    syscall

    addiu   $a0,    $zero,  1   # bios_uart_str
    lui     $a1,    %hi(LOADER_INODE_TABLE_FOR_ROOT_LOADED)
    addiu   $a1,    $a1,    %lo(LOADER_INODE_TABLE_FOR_ROOT_LOADED)
    syscall

    # load root directory to 0xC0000C00
    addiu   $a0,    $zero,  8   # bios_load_file_by_inode
    lui     $at,    0xC000
    addiu   $a1,    $at,    2176
    addiu   $a2,    $at,    3096
    lui     $a3,    %hi(sector_start)
    lw      $a3,    %lo(sector_start)($a3)
    syscall

    addiu   $a0,    $zero,  1   # bios_uart_str
    lui     $a1,    %hi(LOADER_ROOT_DIR_LOADED)
    addiu   $a1,    $a1,    %lo(LOADER_ROOT_DIR_LOADED)
    syscall

    # search kernel.img
    addiu   $a0,    $zero,  10  # bios_find_file_in_dir
    lui     $a1,    0xC000
    addiu   $a1,    $a1,    3096
    lui     $a2,    %hi(KERNEL_NAME)
    addiu   $a2,    $a2,    %lo(KERNEL_NAME)
    syscall

    lui     $at,    %hi(kernel_inode)
    sw      $v0,    %lo(kernel_inode)($at)

    addiu   $a0,    $zero,  1   # bios_uart_str
    lui     $a1,    %hi(LOADER_KERNEL_FOUND)
    addiu   $a1,    $a1,    %lo(LOADER_KERNEL_FOUND)
    syscall

    addiu   $a0,    $zero,  2   # bios_uart_hex
    lui     $at,    %hi(kernel_inode)
    lw      $a1,    %lo(kernel_inode)($at)
    syscall

    addiu   $a0,    $zero,  1   # bios_uart_str
    lui     $a1,    %hi(LOADER_RETURN_LINE)
    addiu   $a1,    $a1,    %lo(LOADER_RETURN_LINE)
    syscall

    lui     $k0,    %hi(kernel_inode)
    lw      $k0,    %lo(kernel_inode)($k0)
    addiu   $k0,    $k0,    -1
    srl     $k0,    $k0,    3
    # load block contains kernel's inode to 0xC0000000
    addiu   $a0,    $zero,  6   # bios_block_read
    lui     $at,    %hi(sector_start)
    lw      $a1,    %lo(sector_start)($at)
    lui     $at,    0xC000
    lw      $a2,    1032($at)
    addu    $a2,    $a2,    $k0
    addiu   $a3,    $at,    0
    syscall

    addiu   $a0,    $zero,  1   # bios_uart_str
    lui     $a1,    %hi(LOADER_INODE_TABLE_FOR_KERNEL_LOADED)
    addiu   $a1,    $a1,    %lo(LOADER_INODE_TABLE_FOR_KERNEL_LOADED)
    syscall

    lui     $k0,    %hi(kernel_inode)
    lw      $k0,    %lo(kernel_inode)($k0)
    addiu   $k0,    $k0,    -1
    andi    $k0,    $k0,    7
    sll     $k0,    $k0,    7
    # load kernel.img to 0xC0080000
    addiu   $a0,    $zero,  8   # bios_load_file_by_inode
    lui     $a1,    0xC000
    addu    $a1,    $a1,    $k0
    lui     $a2,    0xC010
    lui     $a3,    %hi(sector_start)
    lw      $a3,    %lo(sector_start)($a3)
    syscall

    lui     $at,    0xC000
    sw      $v0,    0($at)

    addiu   $a0,    $zero,  1   # bios_uart_str
    lui     $a1,    %hi(LOADER_KERNEL_LOADED)
    addiu   $a1,    $a1,    %lo(LOADER_KERNEL_LOADED)
    syscall

    addiu   $a0,    $zero,  1   # bios_uart_str
    lui     $a1,    %hi(LOADER_KERNEL_SIZE)
    addiu   $a1,    $a1,    %lo(LOADER_KERNEL_SIZE)
    syscall

    addiu   $a0,    $zero,  2   # bios_uart_hex
    lui     $at,    0xC000
    lw      $a1,    0($at)
    syscall

    addiu   $a0,    $zero,  1   # bios_uart_str
    lui     $a1,    %hi(LOADER_RETURN_LINE)
    addiu   $a1,    $a1,    %lo(LOADER_RETURN_LINE)
    syscall

    # jump to kernel
    lui     $gp,    0xC010
    jr      $gp

forever:
    j       forever

    .section ".rodata"
KERNEL_NAME:
    .asciiz "kernel.img"

LOADER_START:
    .asciiz "loader starting\n"
LOADER_TOTAL_INODES:
    .asciiz "superblock loaded\nmagic number: "
LOADER_GROUP_TABLE_LOADED:
    .asciiz "group table loaded\n"
LOADER_INODE_TABLE_FOR_ROOT_LOADED:
    .asciiz "inode table for root loaded\n"
LOADER_INODE_TABLE_FOR_KERNEL_LOADED:
    .asciiz "inode table for kernel loaded\n"
LOADER_ROOT_DIR_LOADED:
    .asciiz "root dir loaded\n"
LOADER_KERNEL_FOUND:
    .asciiz "kernel found at: "
LOADER_KERNEL_LOADED:
    .asciiz "kernel loaded\n"
LOADER_KERNEL_SIZE:
    .asciiz "kernel size: "
LOADER_RETURN_LINE:
    .asciiz "\n"

    .data
sector_start:
    .4byte  (0)
kernel_inode:
    .4byte  (0)
