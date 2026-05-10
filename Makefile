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

all: $(TARGET)
	

$(TARGET): $(OBJS)
	@$(CC) $(CFLAGS) -o $@ $<
	cp $(TARGET) disk/EFI/BOOT/BOOTX64.EFI

-include $(DEPENDS)

clean:
	rm -rf $(TARGET) *.efi bootloader/src/*.o bootloader/src/*.d

run:
	qemu-system-x86_64 \
	-drive if=pflash,format=raw,readonly=on,file=OVMF.fd \
	-drive format=raw,file=fat:rw:disk \
	-net none