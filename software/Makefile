all:
	make -C kernel
	make -C boot
	cp boot/product/bios.coe ./
	../utils/build-disk.sh
	sudo dd if=boot/product/boot.bin of=/dev/sdb bs=446 count=1 conv=notrunc
	sudo dd if=boot/product/loader.bin of=/dev/sdb1 bs=1024 count=1 conv=notrunc
	sudo mount /dev/sdb1 /mnt
	sudo cp kernel/product/kernel.img /mnt
	sudo umount /mnt

run: all
	~/c-stack/mips_c/build/mips_c -l boot/product/bios.bin:4294963200 -s 4294963200 -f /dev/sdb

bin: all
	sudo dd if=/dev/sdb of=system.bin bs=4M count=8

.PHONY: all clean run

clean:
	rm -f bios.coe
	make -C boot clean
	make -C kernel clean
	make -C libc clean
