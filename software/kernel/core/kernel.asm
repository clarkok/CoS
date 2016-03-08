    .text
pic_init:
    lui     $t0,    %hi(pic_mm_init)
    lui     $t1,    0x4000
    addiu   $t0,    $t0,    %lo(pic_mm_init)
    addu    $t0,    $t0,    $t1

    jalr    $ra,    $t0     # calling pic_mm_init

    lui     $sp,    0xC000
    jal     init

__assert_fail:
    break
