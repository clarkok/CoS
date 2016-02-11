all:
	make -C kernel
	make -C boot
	cp boot/product/bios.hex ./
	../utils/build-disk.sh
	sudo dd if=boot/product/boot.bin of=/dev/sdb bs=446 count=1 conv=notrunc
	sudo dd if=boot/product/loader.bin of=/dev/sdb bs=1024 count=1 seek=31 conv=notrunc
	sudo mount /dev/sdb1 /mnt
	sudo cp kernel/product/kernel.img /mnt
	sudo umount /mnt

.PHONY: all clean

clean:
	rm bios.hex
	make -C boot clean
	make -C kernel clean
