OUTPUT := rats

NASM := nasm
CC := clang
LD := ld.lld

HOST_CC := clang
HOST_CFLAGS := -g -O2 -pipe
HOST_CPPFLAGS :=
HOST_LDFLAGS :=
HOST_LIBS :=

DEPS_PATHS := limine-protocol/include flanterm/src freestanding-c-headers/include nanoprintf

DEPS_INCLUDES := $(addprefix -I deps/,$(DEPS_PATHS))

NASMFLAGS := \
	-g -F dwarf -f elf64
CFLAGS := \
	-Wall -Wextra -std=c99 -g -O0 \
	-nostdinc -ffreestanding -fno-stack-protector -fno-lto -fno-PIC \
	-target x86_64-unknown-none-elf -m64 -march=x86-64 -mabi=sysv -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel \
	-I src $(DEPS_INCLUDES) -MMD -MP \
    -ffunction-sections \
    -fdata-sections
LDFLAGS := \
	-m elf_x86_64 -T linker.ld \
	-nostdlib -static -z max-page-size=0x1000 

QEMU_FLAGS := -m 2G --no-reboot --no-shutdown -serial stdio

OUTPUT_BIN := $(OUTPUT).bin
OUTPUT_ISO := $(OUTPUT).iso

CSRC := $(shell find src deps/cc-runtime/src deps/flanterm/src -type f -name '*.c')
ASMSRC := $(shell find src -type f -name '*.asm')
COBJ := $(addprefix build/,$(addsuffix .o,$(CSRC)))
ASMOBJ := $(addprefix build/,$(addsuffix .o,$(ASMSRC)))

.PHONY: clean
.PHONY: distclean

all: $(OUTPUT_BIN)

$(COBJ): build/%.c.o: %.c
	@mkdir -p "$(dir $@)"
	$(CC) -c "$<" -o "$@" $(CFLAGS)

$(ASMOBJ): build/%.asm.o: %.asm
	@mkdir -p "$(dir $@)"
	$(NASM) "$<" -o "$@" $(NASMFLAGS)

$(OUTPUT_BIN): $(COBJ) $(ASMOBJ) linker.ld
	$(LD) -o "$@" $(COBJ) $(ASMOBJ) $(LDFLAGS)

$(OUTPUT_ISO): $(OUTPUT_BIN) deps/limine/limine limine.conf
	rm -rf build/iso
	mkdir -p build/iso/boot/limine
	mkdir -p build/iso/EFI/BOOT
	cp $(OUTPUT_BIN) build/iso/boot/
	cp limine.conf build/iso/boot/limine
	cp deps/limine/limine-bios.sys deps/limine/limine-bios-cd.bin deps/limine/limine-uefi-cd.bin build/iso/boot/limine/
	cp deps/limine/BOOTX64.EFI deps/limine/BOOTIA32.EFI build/iso/EFI/BOOT
	xorriso -as mkisofs -rock -rational-rock -joliet -eltorito-boot boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
		-apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		build/iso -o $(OUTPUT_ISO)
	./deps/limine/limine bios-install $(OUTPUT_ISO)

deps/limine/limine:
	rm -rf deps/limine/
	git clone https://github.com/Limine-Bootloader/Limine.git deps/limine --branch=v11.3.1-binary --depth=1
	$(MAKE) -C deps/limine \
		CC="$(HOST_CC)" \
		CFLAGS="$(HOST_CFLAGS)" \
		CPPFLAGS="$(HOST_CPPFLAGS)" \
		LDFLAGS="$(HOST_LDFLAGS)" \
		LIBS="$(HOST_LIBS)"

edk2-ovmf:
	curl --skip-existing --location https://github.com/osdev0/edk2-ovmf-nightly/releases/latest/download/edk2-ovmf.tar.gz | gunzip | tar -xf -

run: edk2-ovmf $(OUTPUT_ISO)
	qemu-system-x86_64 \
		-machine q35 \
		-drive if=pflash,unit=0,format=raw,file=edk2-ovmf/ovmf-code-x86_64.fd,readonly=on \
		-cdrom $(OUTPUT_ISO) \
		$(QEMU_FLAGS)

debug: edk2-ovmf $(OUTPUT_ISO)
	qemu-system-x86_64 \
		-machine q35 \
		-drive if=pflash,unit=0,format=raw,file=edk2-ovmf/ovmf-code-x86_64.fd,readonly=on \
		-cdrom $(OUTPUT_ISO) \
		-s -S -d int,cpu_reset \
		$(QEMU_FLAGS)

clean:
	rm -rf $(OUTPUT_BIN) build/

distclean:
	rm -rf $(OUTPUT_BIN) build/ edk2-ovmf deps/limine/
