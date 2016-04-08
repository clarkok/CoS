SOURCES	= string.c ctype.c printf.c

DEP_DIR = build/dep/
OBJ_DIR = build/
SRC_DIR = ./

include ../../Makefile.inc

product/libc.bc: $(OBJECTS)
	$(LK) -o $@ $^

LIBC_CC = $(CC) $(LIBC_FLAGS)

$(foreach i,$(SOURCES),$(eval $(call COMPILE_MACRO,$(i),$(LIBC_CC))))

-include $(DEP_DIR)*.P

.PHONY: clean asm obj

asm: product/libc.bc
	$(ASW) -o product/libc.asm product/libc.bc

obj: product/libc.bc
	$(ASW) -filetype=obj -o product/libc.o product/libc.bc

clean:
	rm build/*.bc
	rm build/dep/*
	rm product/*
