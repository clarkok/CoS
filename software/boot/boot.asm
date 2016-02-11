    .text
boot_init:
    addiu   $a0,    $zero,  1   # bios_uart_str
    lui     $a1,    %hi(BOOT_START)
    addiu   $a1,    $a1,    %lo(BOOT_START)
    syscall

    addiu   $a0,    $zero,  7   # bios_load_unaligned_word
    addiu   $a1,    $gp,    454 # first sector of #1 partition
    syscall

    addiu   $at,    $gp,    512
    sw      $v0,    0($at)

    addiu   $a0,    $zero,  1   # bios_uart_str
    lui     $a1,    %hi(BOOT_FIRST_SECTOR)
    addiu   $a1,    $a1,    %lo(BOOT_FIRST_SECTOR)
    syscall

    addiu   $at,    $gp,    512
    addiu   $a0,    $zero,  2   # bios_uart_hex
    lw      $a1,    0($at)
    syscall

    addiu   $a0,    $zero,  1   # bios_uart_str
    lui     $a1,    %hi(BOOT_RETURN_LINE)
    addiu   $a1,    $a1,    %lo(BOOT_RETURN_LINE)
    syscall

    addiu   $at,    $gp,    512
    addiu   $a0,    $zero,  6   # bios_block_read
    lw      $a1,    0($at)
    move    $a2,    $zero
    addiu   $a3,    $gp,    4096
    syscall

    addiu   $a0,    $zero,  1   # bios_uart_str
    lui     $a1,    %hi(BOOT_JUMP)
    addiu   $a1,    $a1,    %lo(BOOT_JUMP)
    syscall

    addiu   $gp,    $gp,    4096
    sync
    jr      $gp

# forever
    addiu   $a0,    $zero,  1
    lui     $a1,    %hi(BOOT_FOREVER)
    addiu   $a1,    $a1,    %lo(BOOT_FOREVER)
    syscall
forever:
    j       forever

    .section ".rodata"
BOOT_START:
    .asciiz "boot starting\nIt seems that memory and flash is good!\n"
BOOT_FIRST_SECTOR:
    .asciiz "first sector: "
BOOT_RETURN_LINE:
    .asciiz "\n"
BOOT_JUMP:
    .asciiz "boot jumping to loader\n"
BOOT_FOREVER:
    .asciiz "boot forever\n"
