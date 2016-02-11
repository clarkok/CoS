CORE_ASM= core/kernel.asm

DEP_DIR = build/dep/
OBJ_DIR = build/
SRC_DIR = ./

CORE_SRC	= core/init.c core/debug.c
MM_SRC		= mm/mm.c
UTILS_SRC	= utils/bitmap.c
SOURCES = $(CORE_SRC) $(MM_SRC) $(UTILS_SRC)

include ../../Makefile.inc

product/kernel.img: $(CORE_ASM) build/kernel.asm
	$(AS) -l 274432 -o $@ -O 2148532224 -m product/kernel.map $^

build/kernel.asm: build/kernel.bc
	$(ASW) -o $@ $<

build/kernel.bc: $(OBJECTS)
	$(LK) -o $@ $^

$(foreach i,$(SOURCES),$(eval $(call COMPILE_MACRO,$(i))))

-include $(DEP_DIR)*.P

.PHONY: clean

clean:
	rm product/*
