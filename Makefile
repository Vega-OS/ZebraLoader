define DEFAULT_VAR =
    ifeq ($(origin $1),default)
        override $(1) := $(2)
    endif
    ifeq ($(origin $1),undefined)
        override $(1) := $(2)
    endif
endef

$(eval $(call DEFAULT_VAR,CC,cc))
$(eval $(call DEFAULT_VAR,LD,ld))
$(eval $(call DEFAULT_VAR,OBJCOPY,objcopy))

CFLAGS ?= -g -O2 -pipe -Wall -Wextra
LDFLAGS ?=

OUTPUT_IMG = Zebra.img
KERNEL_ELF = vega-kernel
C_COMPILER = cross/bin/x86_64-elf-gcc
LD_PATH = cross/bin/x86_64-elf-ld

override INTERNALLDFLAGS :=                \
    -Tlimine-efi/gnuefi/elf_x86_64_efi.lds \
    -nostdlib                              \
    -z max-page-size=0x1000                \
    -m elf_x86_64                          \
    -static                                \
    -pie                                   \
    --no-dynamic-linker                    \
    -z text

override INTERNALCFLAGS :=  \
    -std=gnu11              \
    -ffreestanding          \
    -fno-stack-protector    \
    -fno-stack-check        \
    -fshort-wchar           \
    -fno-lto                \
    -fpie                   \
    -m64                    \
    -march=x86-64           \
    -mabi=sysv              \
    -mno-80387              \
    -mno-mmx                \
    -mno-sse                \
    -mno-sse2               \
    -mno-red-zone           \
    -MMD                    \
    -DGNU_EFI_USE_MS_ABI    \
    -I.                     \
    -Ilimine-efi/inc        \
    -Ilimine-efi/inc/x86_64 \
		-Isrc/include/					\
		-D __KERNEL_ELF='L"$(KERNEL_ELF)"'

override CFILES := $(shell find ./src -type f -name '*.c')
override OBJ := $(CFILES:.c=.o)
override HEADER_DEPS := $(CFILES:.c=.d)

.PHONY: all
all: BOOTX64.EFI

limine-efi:
	- git clone https://github.com/limine-bootloader/limine-efi.git

limine-efi/gnuefi/crt0-efi-x86_64.o limine-efi/gnuefi/reloc_x86_64.o: limine-efi
	$(MAKE) -C limine-efi/gnuefi ARCH=x86_64

BOOTX64.EFI: zebra.elf
	$(OBJCOPY) -O binary $< $@
	mkdir -p boot/EFI/BOOT
	cp BOOTX64.EFI boot/EFI/BOOT/BOOTX64.EFI
	dd if=/dev/zero of=$(OUTPUT_IMG) bs=512 count=93750
	mformat -i $(OUTPUT_IMG)
	mmd -i $(OUTPUT_IMG) ::/EFI
	mmd -i $(OUTPUT_IMG) ::/EFI/BOOT
	mcopy -i $(OUTPUT_IMG) startup.nsh ::
	mcopy -i $(OUTPUT_IMG) $(KERNEL_ELF) ::
	mcopy -i $(OUTPUT_IMG) BOOTX64.EFI ::/EFI/BOOT/

zebra.elf: limine-efi/gnuefi/crt0-efi-x86_64.o limine-efi/gnuefi/reloc_x86_64.o $(OBJ)
	$(LD_PATH) $^ $(LDFLAGS) $(INTERNALLDFLAGS) -o $@

-include $(HEADER_DEPS)
%.o: %.c limine-efi
	$(C_COMPILER) $(CFLAGS) $(INTERNALCFLAGS) -c $< -o $@

ovmf:
	mkdir -p ovmf
	cd ovmf && curl -o OVMF-X64.zip https://efi.akeo.ie/OVMF/OVMF-X64.zip && unzip OVMF-X64.zip

.PHONY: run
run: all ovmf
	qemu-system-x86_64 --enable-kvm -M q35 -drive file=$(OUTPUT_IMG) -bios ovmf/OVMF.fd -m 256M

.PHONY: clean
clean:
	rm -rf BOOTX64.EFI zebra.elf $(OBJ) $(HEADER_DEPS)

.PHONY: distclean
distclean: clean
	rm -rf limine-efi ovmf
