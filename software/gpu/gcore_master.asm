_init:
    lui     $k0,    0x10
    addi    $t0,    $zero,  1   # color

_main_loop:
    sw      $t0,    3($k0)      # slave_command
    lw      $t1,    4($k0)      # frame_counter
    lw      $t2,    0($k0)      # vga_offset
    beq     $t2,    $zero,  _vram_1
_vram_0:
    addi    $t3,    $zero,  0   # video buffer
    j       _start_draw
_vram_1:
    lui     $t3,    3           # video buffer
_start_draw:
    addi    $t4,    $t3,    0   # saved video buffer
    lui     $t5,    3
    srl     $t5,    $t5,    1
    add     $t3,    $t3,    $t5
    lui     $t2,    3           # length
    srl     $t2,    $t2,    1
    addi    $t7,    $t0,    0
_draw_loop:
    addi    $t2,    $t2,    -1
    sw      $t7,    0($t3)
    addi    $t7,    $t7,    1
    addi    $t3,    $t3,    1
_draw_cmp:
    bne     $t2,    $zero,  _draw_loop
_wait_for_slave:
    lw      $t5,    3($k0)
    bne     $t5,    $zero,  _wait_for_slave

    sw      $t4,    0($k0)
_wait_for_vga:
    lw      $t6,    4($k0)
    beq     $t6,    $t1,    _wait_for_vga
_forever:
    addi    $t0,    $t0,    1
    j       _main_loop
