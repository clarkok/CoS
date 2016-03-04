    .text
bios_init:
    lui     $sp,    0xE000

    lui     $k0,    %hi(bios_handler)
    addiu   $k0,    $k0,    %lo(bios_handler)
    mtc0    $k0,    4

    lw      $t0,    0xFE18($zero)
    andi    $t0,    $t0,    1
    beqz    $t0,    _bios_l1_not_check
    
_bios_l1_check:
    lui     $a1,    0xC000
    addiu   $a2,    $a1,    1024
    jal     bios_memory_check
    beqz    $v0,    bios_l1_check_failed

    lui     $a1,    %hi(BIOS_L1_CHECKED)
    addiu   $a1,    $a1,    %lo(BIOS_L1_CHECKED)
    jal     bios_uart_str

_bios_l1_not_check:
    lw      $t0,    0xFE18($zero)
    andi    $t0,    $t0,    2
    beqz    $t0,    _bios_l2_not_check

_bios_l2_check:
    lui     $a1,    0xC000
    addiu   $a2,    $a1,    16384
    jal     bios_memory_check
    beqz    $v0,    bios_l2_check_failed

    lui     $a1,    %hi(BIOS_L2_CHECKED)
    addiu   $a1,    $a1,    %lo(BIOS_L2_CHECKED)
    jal     bios_uart_str

_bios_l2_not_check:
    lw      $t0,    0xFE18($zero)
    andi    $t0,    $t0,    4
    beqz    $t0,    _bios_memory_not_check

_bios_memory_full_check:
    lui     $a1,    0xC000
    lui     $a2,    0xE000
    jal     bios_memory_check
    beqz    $v0,    bios_memory_check_failed

    lui     $a1,    %hi(BIOS_MEMORY_CHECKED)
    addiu   $a1,    $a1,    %lo(BIOS_MEMORY_CHECKED)
    jal     bios_uart_str

_bios_memory_not_check:
    lw      $t0,    0xFE18($zero)
    andi    $t0,    $t0,    8
    beqz    $t0,    _bios_flash_not_check

_bios_flash_check:
    move    $a1,    $zero
    addiu   $a2,    $zero,  128
    jal     bios_flash_check
    beqz    $v0,    _bios_flash_check_failed

    lui     $a1,    %hi(BIOS_FLASH_CHECKED)
    addiu   $a1,    $a1,    %lo(BIOS_FLASH_CHECKED)
    jal     bios_uart_str

_bios_flash_not_check:

    addiu   $a0,    $zero,  1   # bios_uart_str
    lui     $a1,    %hi(BIOS_START)
    addiu   $a1,    $a1,    %lo(BIOS_START)
    syscall

    addiu   $a0,    $zero,  5   # bios_flash_read
    addiu   $a1,    $zero,  0
    lui     $a2,    0xC000
    syscall

    addiu   $a0,    $zero,  1   # bios_uart_str
    lui     $a1,    %hi(BIOS_JUMP)
    addiu   $a1,    $a1,    %lo(BIOS_JUMP)
    syscall

    lui     $gp,    0xC000
    sync
    jr      $gp

bios_l1_check_failed:
    lui     $a1,    %hi(BIOS_L1_FAILED)
    addiu   $a1,    $a1,    %lo(BIOS_L1_FAILED)
    jal     bios_uart_str
    j       forever

bios_l2_check_failed:
    lui     $a1,    %hi(BIOS_L2_FAILED)
    addiu   $a1,    $a1,    %lo(BIOS_L2_FAILED)
    jal     bios_uart_str
    j       forever

bios_memory_check_failed:
    lui     $a1,    %hi(BIOS_MEMORY_FAILED)
    addiu   $a1,    $a1,    %lo(BIOS_MEMORY_FAILED)
    jal     bios_uart_str
    j       forever

bios_flash_check_failed:
    lui     $a1,    %hi(BIOS_FLASH_FAILED)
    addiu   $a1,    $a1,    %lo(BIOS_FLASH_FAILED)
    jal     bios_uart_str
    j       forever

bios_unknown_exception:
    lui     $a1,    %hi(BIOS_UNKNOWN_EXCEPTION)
    addiu   $a1,    $a1,    %lo(BIOS_UNKNOWN_EXCEPTION)
    jal     bios_uart_str

    lui     $a1,    %hi(BIOS_UNKNOWN_EXCEPTION_CAUSE)
    addiu   $a1,    $a1,    %lo(BIOS_UNKNOWN_EXCEPTION_CAUSE)
    jal     bios_uart_str

    mfc0    $a1,    1
    jal     bios_uart_hex

    lui     $a1,    %hi(BIOS_UNKNOWN_EXCEPTION_PC)
    addiu   $a1,    $a1,    %lo(BIOS_UNKNOWN_EXCEPTION_PC)
    jal     bios_uart_str

    mfc0    $a1,    0
    jal     bios_uart_hex

    lui     $a1,    %hi(BIOS_UNKNOWN_EXCEPTION_PFA)
    addiu   $a1,    $a1,    %lo(BIOS_UNKNOWN_EXCEPTION_PFA)
    jal     bios_uart_str

    mfc0    $a1,    6
    jal     bios_uart_hex

    lui     $a1,    %hi(BIOS_RETURN_LINE)
    addiu   $a1,    $a1,    %lo(BIOS_RETURN_LINE)
    jal     bios_uart_str

# forever
    addiu   $a0,    $zero,  1   # bios_uart_str
    lui     $a1,    %hi(BIOS_FOREVER)
    addiu   $a1,    $a1,    %lo(BIOS_FOREVER)
    syscall
forever:
    j       forever

bios_handler:
    mfc0    $k0,    1
    addiu   $at,    $zero,  14
    sll     $a0,    $a0,    2
    bne     $k0,    $at,    bios_unknown_exception
    lui     $k0,    %hi(SYSCALL_TABLE)
    addu    $k0,    $k0,    $a0
    lw      $k1,    %lo(SYSCALL_TABLE)($k0)
    jalr    $ra,    $k1
    eret

# no params
bios_reset:
    lui     $a1,    %hi(BIOS_RESET)
    addiu   $a1,    $a1,    %lo(BIOS_RESET)
    jal     bios_uart_str
    j       bios_init

# @param $a1 string to send
bios_uart_str:
    addiu   $k0,    $zero,  0xFE0C  # uart tx
    j       _bios_uart_str_cmp
_bios_uart_str_loop:
    sw      $k1,    0($k0)
    addiu   $a1,    $a1,    1
_bios_uart_str_cmp:
    lbu     $k1,    0($a1)
    bne     $k1,    $zero,  _bios_uart_str_loop
    jr      $ra

# @param $a1 value
bios_uart_hex:
    addiu   $k0,    $zero,  0xFE0C
    lui     $a2,    0xF000
    addiu   $a0,    $zero,  8

    j       _bios_uart_hex_cmp
_bios_uart_hex_loop:
    and     $k1,    $a1,    $a2
    sll     $a1,    $a1,    4
    srl     $k1,    $k1,    28
    slti    $at,    $k1,    0xA
    bnez    $at,    _bios_uart_hex_digit
    addiu   $k1,    $k1,    55
    j       _bios_uart_hex_out
_bios_uart_hex_digit:
    addiu   $k1,    $k1,    48
_bios_uart_hex_out:
    sw      $k1,    0($k0)
_bios_uart_hex_cmp:
    addiu   $a0,    $a0,    -1
    bgez    $a0,    _bios_uart_hex_loop

    jr      $ra

# @param $a1 dst
# @param $a2 src
# @param $a3 size
bios_memcpy_byte:
    j       _bios_memcpy_byte_cmp
_bios_memcpy_byte_loop:
    lb      $at,    0($a2)
    addiu   $a2,    $a2,    1
    addiu   $a3,    $a3,    -1
    sb      $at,    0($a1)
    addiu   $a1,    $a1,    1
_bios_memcpy_byte_cmp:
    bne     $a3,    $zero,  _bios_memcpy_byte_loop
    jr      $ra

# @param $a1 dst
# @param $a2 src
# @param $a3 size
bios_memcpy_word:
    j       _bios_memcpy_word_cmp
_bios_memcpy_word_loop:
    lw      $at,    0($a2)
    addiu   $a2,    $a2,    4
    addiu   $a3,    $a3,    -4
    sw      $at,    0($a1)
    addiu   $a1,    $a1,    4
_bios_memcpy_word_cmp:
    bne     $a3,    $zero,  _bios_memcpy_word_loop
    jr      $ra

# @param $a1 sector
# @param $a2 dst
bios_flash_read:
    addiu   $sp,    $sp,    -4
    sw      $ra,    0($sp)

    lui     $at,    0x4000
    or      $a1,    $a1,    $at
    addiu   $at,    $zero,  0xFE00  # flash ctrl
    sw      $a1,    0($at)
_bios_flash_read_loop:
    lw      $a1,    0($at)
_bios_flash_read_cmp:
    bgez    $a1,    _bios_flash_read_loop   # use ready bit in flash status as sign
    sw      $zero,  0($at)
    move    $a1,    $a2
    addiu   $a2,    $zero,  0xFC00
    addiu   $a3,    $zero,  512
    jal     bios_memcpy_word

    lw      $ra,    0($sp)
    addiu   $sp,    $sp,    4
    jr      $ra

# @param $a1 start
# @param $a2 block
# @param $a3 dst
bios_block_read:
    addiu   $sp,    $sp,    -16
    sw      $ra,    0($sp)
    sw      $a1,    4($sp)
    sw      $a2,    8($sp)
    sw      $a3,    12($sp)

    sll     $at,    $a2,    1
    addu    $a1,    $a1,    $at
    move    $a2,    $a3
    jal     bios_flash_read

    lw      $a1,    4($sp)
    lw      $a2,    8($sp)
    lw      $a3,    12($sp)

    sll     $at,    $a2,    1
    addiu   $at,    $at,    1
    addu    $a1,    $a1,    $at
    addiu   $a2,    $a3,    512
    jal     bios_flash_read

    lw      $ra,    0($sp)
    addiu   $sp,    $sp,    16
    jr      $ra

# @param $a1 addr
# @return $v0 value
bios_load_unaligned_word:
    move    $at,    $a1
    lbu     $v0,    0($at)
    lbu     $a1,    1($at)
    lbu     $a2,    2($at)
    lbu     $a3,    3($at)
    sll     $a1,    $a1,    8
    sll     $a2,    $a2,    16
    sll     $a3,    $a3,    24
    or      $v0,    $v0,    $a1
    or      $v0,    $v0,    $a2
    or      $v0,    $v0,    $a3
    jr      $ra

# @param $a0 block list
# @param $a1 dst
# @param $a2 max block count
# @param $a3 max size
# @param $v0 start
# @return $v0 read blocks
bios_load_file_by_block_list:
    addiu   $sp,    $sp,    -32
    sw      $ra,    0($sp)
    sw      $a0,    4($sp)
    sw      $a1,    8($sp)
    sw      $a2,    12($sp)
    sw      $a3,    16($sp)
    sw      $v0,    20($sp)
    sw      $zero,  24($sp)     # retval

_bios_load_file_by_block_list_loop:
    lw      $at,    12($sp)
    beq     $at,    $zero,  _bios_load_file_by_block_list_end
    lw      $at,    16($sp)
    beqz    $at,    _bios_load_file_by_block_list_end
    bltz    $at,    _bios_load_file_by_block_list_end

    lw      $a1,    20($sp)

    lw      $a2,    4($sp)
    addiu   $at,    $a2,    4
    sw      $at,    4($sp)
    lw      $a2,    0($a2)

    lw      $a3,    8($sp)
    addiu   $at,    $a3,    1024
    sw      $at,    8($sp)

    jal     bios_block_read

    lw      $t0,    12($sp)
    lw      $t1,    16($sp)
    lw      $t2,    24($sp)

    addiu   $t0,    $t0,    -1
    addiu   $t1,    $t1,    -1024
    addiu   $t2,    $t2,    1

    sw      $t0,    12($sp)
    sw      $t1,    16($sp)
    sw      $t2,    24($sp)

    j       _bios_load_file_by_block_list_loop

_bios_load_file_by_block_list_end:
    lw      $ra,    0($sp)
    lw      $v0,    24($sp)
    addiu   $sp,    $sp,    32
    jr      $ra

# @param $a1 inode
# @param $a2 dst
# @param $a3 start
# @return $v0 file_size
bios_load_file_by_inode:
    addiu   $sp,    $sp,    -32
    sw      $ra,    0($sp)  # ra
    sw      $a1,    4($sp)  # inode
    sw      $a2,    8($sp)  # dst
    sw      $a3,    24($sp) # start

    lw      $at,    4($a1)
    sw      $at,    12($sp) # remain
    sw      $at,    16($sp) # ret

    addiu   $a0,    $a1,    40  # direct block list
    move    $a1,    $a2
    addiu   $a2,    $zero,  12
    move    $a3,    $at
    lw      $v0,    24($sp)
    jal     bios_load_file_by_block_list

    lw      $at,    12($sp)
    sll     $v0,    $v0,    10
    subu    $at,    $at,    $v0
    beqz    $at,    _bios_load_file_by_inode_end
    bltz    $at,    _bios_load_file_by_inode_end
    sw      $at,    12($sp)
    lw      $at,    8($sp)
    addu    $at,    $at,    $v0
    sw      $at,    8($sp)

    addiu   $sp,    $sp,    -1024

    lw      $a2,    1028($sp)
    lw      $a1,    1048($sp)
    lw      $a2,    88($a2)
    move    $a3,    $sp
    jal     bios_block_read

    move    $a0,    $sp
    lw      $a1,    1032($sp)
    addiu   $a2,    $zero,  256
    lw      $a3,    1036($sp)
    lw      $v0,    1048($sp)
    jal     bios_load_file_by_block_list
    
    addiu   $sp,    $sp,    1024

_bios_load_file_by_inode_end:
    lw      $v0,    16($sp)
    lw      $ra,    0($sp)
    addiu   $sp,    $sp,    32
    jr      $ra

# @param $a1 a
# @param $a2 b
# @return $v0
bios_strcmp:
    lbu     $k0,    0($a1)
    addiu   $a1,    $a1,    1
    lbu     $k1,    0($a2)
    addiu   $a2,    $a2,    1
    beq     $k0,    $k1,    _bios_strcmp_same
    j       _bios_strcmp_end
_bios_strcmp_same:
    bnez    $k0,    bios_strcmp
_bios_strcmp_end:
    subu    $v0,    $k0,    $k1
    jr      $ra

# @param $a1 dir
# @param $a2 filename
# @return $v0 inode_nr
bios_find_file_in_dir:
    addiu   $sp,    $sp,    -16
    sw      $ra,    0($sp)
    sw      $a1,    4($sp)
    sw      $a2,    8($sp)
    addiu   $at,    $a1,    1024
    sw      $at,    12($sp)     # dir limit

    j       _bios_find_file_in_dir_cmp
_bios_find_file_in_dir_loop:
    lw      $at,    4($sp)
    addiu   $a1,    $at,    8
    lw      $a2,    8($sp)
    jal     bios_strcmp
    beqz    $v0,    _bios_find_file_in_dir_found

    lhu     $k0,    4($at)
    addu    $at,    $at,    $k0
    sw      $at,    4($sp)

_bios_find_file_in_dir_cmp:
    lw      $k0,    4($sp)
    lw      $k1,    12($sp)
    bne     $k0,    $k1,    _bios_find_file_in_dir_loop

    move    $v0,    $zero
    j       _bios_find_file_in_dir_end
_bios_find_file_in_dir_found:
    lw      $v0,    0($at)
_bios_find_file_in_dir_end:
    lw      $ra,    0($sp)
    addiu   $sp,    $sp,    16
    jr      $ra

# @param $a1 dst
# @param $a2 value
# @param $a3 count
bios_memset_word:
    sw      $a2,    0($a1)
    addiu   $a3,    $a3,    -4
    addiu   $a1,    $a1,    4
    bnez    $a3,    bios_memset_word
    jr      $ra

# @param $a1 start
# @param $a2 limit
bios_memory_check:
    move    $s3,    $a1
    move    $s4,    $a1
    lui     $s5,    0x0010
    addiu   $s5,    $s5,    -1
    move    $s6,    $ra
    move    $s7,    $a2

    j       _bios_memory_check_fill_cmp
_bios_memory_check_fill_loop:
    and     $t0,    $s4,    $s5
    bnez    $t0,    _bios_memory_check_fill_not_print

    move    $a1,    $s4
    jal     bios_uart_hex
    lui     $a1,    %hi(BIOS_RETURN_LINE)
    addiu   $a1,    $a1,    %lo(BIOS_RETURN_LINE)
    jal     bios_uart_str
_bios_memory_check_fill_not_print:
    sw      $s4,    0($s4)
    addiu   $s4,    $s4,    4
_bios_memory_check_fill_cmp:
    subu    $t0,    $s4,    $s7
    bltz    $t0,    _bios_memory_check_fill_loop

    addiu   $v0,    $zero,  1
    move    $s4,    $s3
    j       _bios_memory_check_verf_cmp
_bios_memory_check_verf_loop:
    and     $t0,    $s4,    $s5
    bnez    $t0,    _bios_memory_check_verf_not_print

    move    $a1,    $s4
    jal     bios_uart_hex
    lui     $a1,    %hi(BIOS_RETURN_LINE)
    addiu   $a1,    $a1,    %lo(BIOS_RETURN_LINE)
    jal     bios_uart_str
_bios_memory_check_verf_not_print:
    lw      $t0,    0($s4)
    bne     $t0,    $s4,    _bios_memory_check_failed
    addiu   $s4,    $s4,    4
_bios_memory_check_verf_cmp:
    subu    $t0,    $s4,    $s7
    bltz    $t0,    _bios_memory_check_verf_loop

    jr      $s6

_bios_memory_check_failed:
    move    $a1,    $t0
    jal     bios_uart_hex
    move    $a1,    $s4
    jal     bios_uart_hex
    lui     $a1,    %hi(BIOS_RETURN_LINE)
    addiu   $a1,    $a1,    %lo(BIOS_RETURN_LINE)
    jal     bios_uart_str
    move    $v0,    $zero
    jr      $s6

# @param $a1 start
# @param $a2 count
bios_flash_check:
    move    $s7,    $ra
    move    $s6,    $a2
    move    $s5,    $a1
    move    $s4,    $a2
    move    $s3,    $a1

    j       _bios_flash_check_fill_cmp
_bios_flash_check_fill_loop:
    move    $a1,    $s3
    jal     bios_uart_hex
    lui     $a1,    %hi(BIOS_RETURN_LINE)
    addiu   $a1,    $a1,    %lo(BIOS_RETURN_LINE)
    jal     bios_uart_str

    sll     $t0,    $s4,    16
    addiu   $t1,    $zero,  128
    addiu   $t2,    $zero,  0xFC00

    j       _bios_flash_check_fill_sector_cmp
_bios_flash_check_fill_sector_loop:
    sw      $t0,    0($t2)
    addiu   $t0,    $t0,    4
    addiu   $t1,    $t1,    -1
    addiu   $t2,    $t2,    4
_bios_flash_check_fill_sector_cmp:
    bnez    $t1,    _bios_flash_check_fill_sector_loop

    lui     $t0,    0x2000
    addu    $t0,    $t0,    $s3
    addiu   $t1,    $zero,  0xFE00
    sw      $t0,    0($t1)

_bios_flash_check_fill_wait_loop:
    lw      $t0,    0($t1)
    bgez    $t0,    _bios_flash_check_fill_wait_loop

    addiu   $s3,    $s3,    1
    addiu   $s4,    $s4,    -1

_bios_flash_check_fill_cmp:
    bnez    $s4,    _bios_flash_check_fill_loop

    move    $s4,    $s6
    move    $s3,    $s5
    j       _bios_flash_check_verf_cmp

_bios_flash_check_verf_loop:
    move    $a1,    $s3
    jal     bios_uart_hex
    lui     $a1,    %hi(BIOS_RETURN_LINE)
    addiu   $a1,    $a1,    %lo(BIOS_RETURN_LINE)
    jal     bios_uart_str
    
    lui     $t0,    0x4000
    addu    $t0,    $t0,    $s3
    addiu   $t1,    $zero,  0xFE00
    sw      $t0,    0($t1)

_bios_flash_check_verf_wait_loop:
    lw      $t0,    0($t1)
    bgez    $t0,    _bios_flash_check_verf_wait_loop

    sll     $t0,    $s4,    16
    addiu   $t1,    $zero,  128
    addiu   $t2,    $zero,  0xFC00

    j       _bios_flash_check_verf_sector_cmp
_bios_flash_check_verf_sector_loop:
    lw      $t3,    0($t2)
    bne     $t3,    $t0,    _bios_flash_check_failed
    addiu   $t0,    $t0,    4
    addiu   $t1,    $t1,    -1
    addiu   $t2,    $t2,    4
_bios_flash_check_verf_sector_cmp:
    bnez    $t1,    _bios_flash_check_verf_sector_loop

    addiu   $s4,    $s4,    -1
    addiu   $s3,    $s3,    1
    
_bios_flash_check_verf_cmp:
    bnez    $s4,    _bios_flash_check_verf_loop

    addiu   $v0,    $zero,  1
    jr      $s7

_bios_flash_check_failed:
    move    $a1,    $t3
    jal     bios_uart_hex
    move    $a1,    $t0
    jal     bios_uart_hex
    lui     $a1,    %hi(BIOS_RETURN_LINE)
    addiu   $a1,    $a1,    %lo(BIOS_RETURN_LINE)
    jal     bios_uart_str

    move    $v0,    $zero
    jr      $s7

    .section ".rodata"
SYSCALL_TABLE:
    .4byte  (bios_reset)
    .4byte  (bios_uart_str)
    .4byte  (bios_uart_hex)
    .4byte  (bios_memcpy_byte)
    .4byte  (bios_memcpy_word)
    .4byte  (bios_flash_read)
    .4byte  (bios_block_read)
    .4byte  (bios_load_unaligned_word)
    .4byte  (bios_load_file_by_inode)
    .4byte  (bios_strcmp)
    .4byte  (bios_find_file_in_dir)
    .4byte  (bios_memset_word)

BIOS_START:
    .asciiz "bios starting\n"
BIOS_JUMP:
    .asciiz "bios jumping to boot\n"
BIOS_FOREVER:
    .asciiz "bios forever\n"
BIOS_UNKNOWN_EXCEPTION:
    .asciiz "unknown exception occurred\n"
BIOS_UNKNOWN_EXCEPTION_CAUSE:
    .asciiz "  cause: "
BIOS_UNKNOWN_EXCEPTION_PC:
    .asciiz "  epc: "
BIOS_UNKNOWN_EXCEPTION_PFA:
    .asciiz "  pfa: "
BIOS_RETURN_LINE:
    .asciiz "\n"
BIOS_RESET:
    .asciiz "bios reset\n"
BIOS_L1_CHECKED:
    .asciiz "bios l1 checked\n"
BIOS_L1_FAILED:
    .asciiz "bios l1 failed\n"
BIOS_L2_CHECKED:
    .asciiz "bios l2 checked\n"
BIOS_L2_FAILED:
    .asciiz "bios l2 failed\n"
BIOS_MEMORY_CHECKED:
    .asciiz "bios memory checked\n"
BIOS_MEMORY_FAILED:
    .asciiz "bios memory failed\n"
BIOS_FLASH_CHECKED:
    .asciiz "bios flash checked\n"
BIOS_FLASH_FAILED:
    .asciiz "bios flash failed\n"
BIOS_END:
