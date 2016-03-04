CORE_ASM= 

DEP_DIR = build/dep/
OBJ_DIR = build/
SRC_DIR = ./

CORE_SRC 	= core/cos.c
UTILS_SRC	= utils/sb-tree.c
SOURCES = $(UTILS_SRC)

include ../../Makefile.inc

product/kernel.img: $(CORE_ASM) build/kernel.asm
	$(AS) -l 274432 -o $@ -O 2148532224 -m product/kernel.map $^

build/kernel.asm: build/kernel.bc
	$(ASW) -o $@ $<

build/kernel.bc: $(OBJECTS) ../libc/product/libc.bc
	make -C ../libc
	$(LK) -o $@ $^

build/kernel-test.bc: $(OBJECTS)
	$(LK) -o $@ $^

$(foreach i,$(SOURCES),$(eval $(call COMPILE_MACRO,$(i))))

-include $(DEP_DIR)*.P

.PHONY: clean test

test: build/kernel-test.bc test/test.c
	$(TEST_CC) -Ithird_party/cutest/ -o test/test test/test.c third_party/cutest/CuTest.c $(SOURCES)
	./test/test

clean:
	rm -f build/*.*
	rm -f product/*
