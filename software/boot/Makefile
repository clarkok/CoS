include ../../Makefile.inc

all: product/bios.coe product/boot.bin product/loader.bin

product/bios.bin: bios.asm
	$(AS) -m product/bios.map -l 4096 -O 4294963200 -o $@ $<

product/boot.bin: boot.asm
	$(AS) -m product/boot.map -l 446 -O 3221225472 -o $@ $<

product/loader.bin: loader.asm
	$(AS) -m product/loader.map -l 1024 -O 3221229568 -o $@ $<

product/bios.coe: product/bios.bin
	$(BIN2COE) $< > $@

.PHONY: clean all write_bootloader

clean:
	rm -f product/*
