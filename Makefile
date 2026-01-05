# Toolchain
CC := aarch64-elf-gcc
LD := aarch64-elf-ld
OBJCOPY := aarch64-elf-objcopy

# Flags
CFLAGS := -Wall -Wextra -O2 -ffreestanding -nostdlib -nostartfiles -fno-stack-protector -mstrict-align -mcpu=cortex-a53+nofp -fno-builtin -U_FORTIFY_SOURCE
LDFLAGS := -nostdlib

# Sources
C_SRCS := $(wildcard drivers/*/*.c kernel/*.c utils/*.c libs/*.c libs/fat32/fat.c)
S_SRCS := $(wildcard drivers/*/*.S boot/*.S libs/*.S)
PSF_SRCS := $(wildcard font/*.psf)

# Objects
C_OBJS := $(C_SRCS:.c=.o)
S_OBJS := $(S_SRCS:.S=.o)
PSF_OBJS := $(PSF_SRCS:.psf=.o)

OBJS := $(C_OBJS) $(S_OBJS) $(PSF_OBJS)

# Targets
all: kernel8.img

kernel8.img: kernel8.elf
	$(OBJCOPY) -O binary $< $@

kernel8.elf: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -T script/link.ld -o $@

# Pattern rules
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.psf
	$(LD) -r -b binary -o $@ $<

run:
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -drive file=disk.img,if=sd,format=raw -serial stdio -d int -no-reboot

debug:
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -drive file=disk.img,if=sd,format=raw -serial stdio -s -S -d int -no-reboot & gnome-terminal -- gdb-multiarch kernel8.elf -ex "target remote :1234" -ex "layout split"

clean:
	rm -f kernel8.elf kernel8.img $(OBJS)
