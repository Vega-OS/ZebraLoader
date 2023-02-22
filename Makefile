CC = cross/bin/x86_64-elf-gcc
LD = cross/bin/x86_64-elf-ld
OBJCOPY = cross/bin/x86_64-elf-objcopy
LD_FLAGS =  -Lgnu-efi/x86_64/lib 											\
						-Lgnu-efi/x86_64/gnuefi										\
						-Tgnu-efi/gnuefi/elf_x86_64_efi.lds				\
						gnu-efi/x86_64/gnuefi/crt0-efi-x86_64.o		\
    				-nostdlib                             		\
						-z max-page-size=0x1000               		\
    				-m elf_x86_64                         		\
    				-static                               		\
    				-pie                                  		\
    				--no-dynamic-linker                   		\
    				-z text	

CFLAGS = -fpic -ffreestanding -fno-stack-protector -fno-stack-check \
				 -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c  \
				 -MMD -Ignu-efi/inc/ -Iinclude/

CFILES = $(shell find src/ -type f -name '*.c')
HEADER_DEPS = $(CFILES:.c=.d)
OBJ = $(CFILES:.c=.o)

OUTPUT = Zebra.img

.PHONY: all
all: $(OUTPUT)

$(OUTPUT): BOOTX64.EFI
	mkdir -p boot/EFI/BOOT
	cp BOOTX64.EFI boot/EFI/BOOT/BOOTX64.EFI
	dd if=/dev/zero of=$@ bs=512 count=93750
	mformat -i $@
	mmd -i $@ ::/EFI
	mmd -i $@ ::/EFI/BOOT
	mcopy -i $@ conf/startup.nsh ::
	mcopy -i $@ assets/wallpaper.bmp ::
	mcopy -i $@ BOOTX64.EFI ::/EFI/BOOT/
	rm BOOTX64.EFI
	rm zebra.elf

BOOTX64.EFI: zebra.elf
	@ #$(OBJCOPY) -O binary $< $@
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym  \
		-j .rel -j .rela -j .rel.* -j .rela.* -j .reloc \
		--target efi-app-x86_64 --subsystem=10 $^ $@

zebra.elf: $(OBJ)
	ld $(LD_FLAGS) $^ -o $@ -lgnuefi -lefi

gnu-efi:
	git clone https://git.code.sf.net/p/gnu-efi/code gnu-efi
	cd gnu-efi; make

-include $(HEADER_DEPS)
%.o: %.c gnu-efi
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm $(HEADER_DEPS) $(OBJ)

.PHONY: run
run: $(OUTPUT)
	qemu-system-x86_64 --enable-kvm -M q35 -cdrom $^ -bios ovmf/OVMF.fd -m 256M
