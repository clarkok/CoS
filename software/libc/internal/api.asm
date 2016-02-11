    .text
    .text
    .globl cos_output_string
    .ent cos_output_string
cos_output_string:
    addi    $v0, $zero, 10
    syscall
    jr      $ra
    .end cos_output_string
