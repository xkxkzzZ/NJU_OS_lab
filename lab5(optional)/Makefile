QEMU = qemu-system-i386

os.img:
	@cd utils/genFS; make
	@cd bootloader; make
	@cd kernel; make
	@cd app; make
	cat bootloader/bootloader.bin kernel/kMain.elf app/fs.bin > os.img

play: os.img
	$(QEMU) -serial stdio os.img

debug: os.img
	$(QEMU) -serial stdio -s -S os.img

clean:
	@cd utils/genFS; make clean
	@cd bootloader; make clean
	@cd kernel; make clean
	@cd app; make clean
	rm -f os.img
