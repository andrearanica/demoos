CC := aarch64-elf-gcc
AS := aarch64-elf-as
LD := aarch64-elf-ld
OBJCOPY := aarch64-elf-objcopy

CFLAGS := -Wall -Wextra -O2 -ffreestanding -nostdlib -nostartfiles
LDFLAGS := -nostdlib

all: kernel8.img

kernel8.img: kernel8.elf
	$(OBJCOPY) -O binary kernel8.elf kernel8.img

kernel8.elf: boot/start.o drivers/framebuffer.o drivers/mbox.o drivers/uart.o font/font.o kernel/kernel.o
	$(LD) $(LDFLAGS) boot/start.o drivers/framebuffer.o drivers/mbox.o drivers/uart.o font/font.o kernel/kernel.o -T script/link.ld -o kernel8.elf

start.o: start.S
	$(CC) $(CFLAGS) -c start.S -o start.o

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.psf
	$(LD) -r -b binary -o $@ $<

run:
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial stdio

clean:
	rm kernel8.elf kernel8.img boot/start.o drivers/framebuffer.o drivers/mbox.o drivers/uart.o font/font.o kernel/kernel.o >/dev/null 2>/dev/null || true

