file ~/c-stack/mips_c/build/mips_c
break mips_c::CPU::nextStep
run -l boot/product/bios.bin:4294963200 -s 4294963200 -f /dev/sdb
disp pc_
disp inst
disp rd
disp rt
