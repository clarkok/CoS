    .text
    add     $t0,    $t1,    $t2
    addi    $t0,    $t1,    1234
    sub     $at,    $zero,  $zero
    ori     $s0,    $zero,  0x2134
    .section .data
tmp:
    .4byte  (tmp)
