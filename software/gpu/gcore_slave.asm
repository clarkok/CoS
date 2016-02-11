_init:
    lui     $k0,    0x10
_main_loop:

_wait_for_master:
    lw      $t0,    3($k0)      # slave_command
    beq     $t0,    $zero,  _wait_for_master

    lw      $t1,    0($k0)      # vga_offset
    beq     $t1,    $zero,  _vram_1
_vram_0:
    addi    $t2,    $zero,  0
    j       _start_draw
_vram_1:
    lui     $t2,    3

_start_draw:
    nor     $t0,    $zero,  $zero
    addi    $t3,    $zero,  10
_draw_vertical_loop:
    addi    $t4,    $zero,  10
    addi    $t5,    $t2,    10
_draw_hor_loop:
    sw      $t0,    0($t5)
    addi    $t4,    $t4,    -1
    addi    $t5,    $t5,    1
_draw_hor_cmp:
    bne     $t4,    $zero,  _draw_hor_loop
    
    addi    $t3,    $t3,    -1
    addi    $t2,    $t2,    256
_draw_vertical_cmp:
    bne     $t3,    $zero,  _draw_vertical_loop

    sw      $zero,  3($k0)      # done
    j       _main_loop
