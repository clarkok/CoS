CORE_ASM= core/kernel.asm core/interrupt.asm core/syscall_table.asm

DEP_DIR = build/dep/
OBJ_DIR = build/
SRC_DIR = ./

CORE_SRC 	= core/cos.c core/kernel.c
MM_SRC		= mm/mm.c mm/buddy.c mm/slab.c mm/linked-buddy.c mm/shared.c
UTILS_SRC	= utils/sb-tree.c
PROC_SRC	= proc/proc.c proc/init_proc.c proc/message.c
DRIVER_SRC	= driver/driver.c driver/timer.c
SOURCES 	= $(CORE_SRC) $(UTILS_SRC) $(MM_SRC) $(PROC_SRC) $(DRIVER_SRC)

include ../../Makefile.inc

TEST_SRC 	= utils/sb-tree.c mm/buddy.c mm/slab.c mm/linked-buddy.c

KERNEL_CC 	= $(CC) $(KERNEL_FLAGS)

product/kernel.img: $(CORE_ASM) build/kernel.asm
	$(AS) -l 274432 -o $@ -O 2148532224 -m product/kernel.map $^

build/kernel.asm: build/kernel.bc
	$(ASW) -o $@ $<

build/kernel.bc: $(OBJECTS) ../libc/product/libc.bc
	$(LK) -o $@ $^

../libc/product/libc.bc:
	make -C ../libc

build/kernel-test.bc: $(OBJECTS)
	$(LK) -o $@ $^

$(foreach i,$(SOURCES),$(eval $(call COMPILE_MACRO,$(i),$(KERNEL_CC))))

-include $(DEP_DIR)*.P

.PHONY: clean test

test: build/kernel-test.bc test/test.c
	$(TEST_CC) -D__TEST__ -Ithird_party/cutest/ -o test/test test/test.c third_party/cutest/CuTest.c $(TEST_SRC)
	./test/test

clean:
	rm -f build/*.*
	rm -f product/*
