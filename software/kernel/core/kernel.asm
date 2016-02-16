    .text
kernel_init:
    addiu   $a0,    $zero,  1   # bios_uart_str
    lui     $a1,    %hi(KERNRL_START)
    lui     $at,    0x4000
    addiu   $a1,    $a1,    %lo(KERNRL_START)
    addu    $a1,    $a1,    $at
    syscall

    # get kernel size from loader.asm
    lui     $at,    0xC000
    lw      $s0,    0($at)
    lui     $at,    %hi(kernel_size)
    lui     $t0,    0x4000
    addu    $at,    $at,    $t0
    sw      $s0,    %lo(kernel_size)($at)

    # clear lower 1MB from 0xC0000000
    addiu   $a0,    $zero,  11
    lui     $a1,    0xC000
    move    $a2,    $zero
    lui     $a3,    0x0010
    syscall

    ## build struct MemoryManaegment at 0xC0000000
    # build page dir at 0xC0000000 + 8KB
    lui     $at,    0xC000
    addiu   $t0,    $zero,  0x3001      # lower 4MB
    sw      $t0,    10240($at)          # from 2GB to 2GB + 4MB

    addiu   $t0,    $zero,  0x4001
    sw      $t0,    11260($at)          # stack

    # build page ent at 0xC0000000 + 12KB
    lui     $at,    %hi(kernel_size)
    lui     $t0,    0x4000
    addu    $at,    $at,    $t0
    lw      $s0,    %lo(kernel_size)($at)
    addiu   $s0,    $s0,    4095
    srl     $s0,    $s0,    12
    addiu   $s0,    $s0,    256
    lui     $s1,    0xC000
    addiu   $s1,    $s1,    12288
    move    $s2,    $zero
    beq     $zero,  $zero,  _kernel_build_page_ent_cmp  # PIC jump
_kernel_build_page_ent_loop:
    sll     $at,    $s2,    12
    addiu   $at,    $at,    3
    sw      $at,    0($s1)
    addiu   $s0,    $s0,    -1
    addiu   $s1,    $s1,    4
    addiu   $s2,    $s2,    1
_kernel_build_page_ent_cmp:
    bnez    $s0,    _kernel_build_page_ent_loop

    # build stack page ent at 0xC0000000 + 16KB
    # use only last 2 pages
    lui     $at,    0xC000
    lui     $t0,    %hi(0x000FE003)
    addiu   $t0,    $t0,    %lo(0x000FE003)
    sw      $t0,    0x4FF8($at)
    addiu   $t0,    $t0,    0x1000
    sw      $t0,    0x4FFC($at)

    # setting dir_bitmap
    addiu   $t0,    $zero,  3
    lui     $at,    0xC000
    sw      $t0,    0($at)

    # setting ent_allocated
    addiu   $t0,    $zero,  251
    lui     $at,    0xC000
    sw      $t0,    4088($at)

    # setting heap_point
    lui     $t0,    0x0010
    lui     $at,    %hi(kernel_size)
    lui     $t1,    0x4000
    addu    $at,    $at,    $t1
    lw      $at,    %lo(kernel_size)($at)
    addu    $t0,    $t0,    $at
    lui     $at,    0xC000
    sw      $t0,    4092($at)

    # initial $sp
    lui     $sp,    0xC000
    
    addiu   $at,    $zero,  8192
    sync
    mtc0    $at,    5
    lui     $t0,    %hi(paging_succeed)
    addiu   $t0,    $t0,    %lo(paging_succeed)
    jr      $t0

paging_succeed:
    addiu   $a0,    $zero,  1   # bios_uart_str
    lui     $a1,    %hi(KERNRL_PAGED)
    addiu   $a1,    $a1,    %lo(KERNRL_PAGED)
    syscall

    jal     init

forever:
    j       forever

    .data
kernel_size:
    .4byte  (0)

    .section ".rodata"
KERNRL_START:
    .asciiz "kernel starting!\n"
KERNRL_PAGED:
    .asciiz "kernel paged!\n"
