.POSIX:
.PHONY: all clean run

SOURCE = bootloader/src/main.c
OBJS = $(SOURCE:.c=.o)
DEPENDS = $(OBJS:.o=.d)
TARGET = BOOTX64.EFI

CC = x86_64-w64-mingw32-gcc \
	-Wl,--subsystem,10 \
	-e efi_main 

CFLAGS = \
	-std=c17 \
	-MMD \
	-Wall \
	-Wextra \
	-Wpedantic \
	-mno-red-zone \
	-ffreestanding \
	-nostdlib 

KERNEL_CC = gcc
KERNEL_LD = ld

KERNEL_CFLAGS = \
	-std=c17 \
	-Wall \
	-Wextra \
	-ffreestanding \
	-fno-stack-protector \
	-fno-pic \
	-fno-pie \
	-mno-red-zone \
	-mcmodel=kernel \
	-O2

KERNEL_LDFLAGS = \
	-nostdlib \
	-z max-page-size=0x1000 \
	-T kernel/linker.ld

KERNEL_SRC = kernel/src/main.c
KERNEL_OBJ = kernel/src/main.o
KERNEL_TARGET = kernel.elf

all: $(TARGET) $(KERNEL_TARGET)
	

$(KERNEL_TARGET): $(KERNEL_OBJ) kernel/linker.ld
	$(KERNEL_LD) $(KERNEL_LDFLAGS) -o $@ $(KERNEL_OBJ)
	cp $(KERNEL_TARGET) disk/kernel.elf

kernel/src/main.o: kernel/src/main.c
	$(KERNEL_CC) $(KERNEL_CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	@$(CC) $(CFLAGS) -o $@ $<
	cp $(TARGET) disk/EFI/BOOT/BOOTX64.EFI

-include $(DEPENDS)

clean:
	rm -rf $(TARGET) $(KERNEL_TARGET) *.efi bootloader/src/*.o bootloader/src/*.d kernel/src/*.o kernel/src/*.d *.elf

run:
	qemu-system-x86_64 \
	-drive if=pflash,format=raw,readonly=on,file=OVMF.fd \
	-drive format=raw,file=fat:rw:disk \
	-net none \
	-serial stdio