
ROOT ?= $(PWD)

CONFIG ?= .config

-include $(CONFIG)

SETTINGS_FILE = $(ROOT)/include/kernel/settings.h

$(shell echo "#define ROOTFS_DEVNO (($(CONFIG_ROOTFS_DEV_MAJ) << 4) | $(CONFIG_ROOTFS_DEV_MIN))" > $(SETTINGS_FILE))
$(shell echo "#define ROOTFS_TYPE \"$(CONFIG_ROOTFS_TYPE)\"" >> $(SETTINGS_FILE))
$(shell echo "#define INIT_PATH \"$(CONFIG_INIT_PATH)\"" >> $(SETTINGS_FILE))
$(shell echo "#define INIT_CONSOLE_DEVNO $(CONFIG_INIT_CONSOLE_DEVNO)" >> $(SETTINGS_FILE))

ifeq ($(CONFIG_ARCH_TAURUS), y)
ARCH = riscv
TOOLCHAIN = riscv64-unknown-elf
ARCH_CFLAGS = -march=rv32i -mabi=ilp32
ARCH_LDFLAGS = -march=rv32i -mabi=ilp32
$(shell echo "#define ARCH_TAURUS" >> $(SETTINGS_FILE))
endif

ifeq ($(CONFIG_ARCH_MCXA153), y)
ARCH = mcxa153
TOOLCHAIN = ~/arm-gnu-toolchain-15.2.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi
ARCH_CFLAGS = -mcpu=cortex-m33 -mthumb -std=gnu11 -DCPU_MCXA153VFM
ARCH_LDFLAGS =
$(shell echo "#define ARCH_MCXA153" >> $(SETTINGS_FILE))
endif

# the filesystems
FS_SELECT = $(ROOT)/fs/fs.c $(ROOT)/fs/vfs.c $(ROOT)/fs/devfs.c

ifeq ($(CONFIG_FS_FATFS), y)
FS_SELECT += $(ROOT)/fs/fatfs.c
$(shell echo "#define FS_FATFS" >> $(SETTINGS_FILE))
endif
ifeq ($(CONFIG_FS_ROMFS), y)
FS_SELECT += $(ROOT)/fs/romfs.c
$(shell echo "#define FS_ROMFS" >> $(SETTINGS_FILE))
endif

DEV_SELECT = 

ifeq ($(CONFIG_CM2_BLOCK_DEV), y)
DEV_SELECT += $(ROOT)/drivers/cm2disk/cm2disk.c
$(shell echo "#define CM2_BLOCK_DEV" >> $(SETTINGS_FILE))
endif

ifeq ($(CONFIG_CM2_TILING_GPU), y)
DEV_SELECT += $(ROOT)/drivers/tilegpu/tilegpu.c
$(shell echo "#define CM2_TILING_GPU" >> $(SETTINGS_FILE))
endif

ifeq ($(CONFIG_TTY_DRIVER), y)
DEV_SELECT += $(ROOT)/drivers/tty/tty.c
$(shell echo "#define TTY_DRIVER" >> $(SETTINGS_FILE))
endif

ifeq ($(CONFIG_USART_DRIVER), y)
DEV_SELECT += $(ROOT)/drivers/USART/usart.c
$(shell echo "#define USART_DRIVER" >> $(SETTINGS_FILE))
endif

ifeq ($(CONFIG_USART_DRIVER_CM2CON), y)
DEV_SELECT += $(ROOT)/drivers/USART/cm2_con.c
$(shell echo "#define USART_DRIVER_CM2CON" >> $(SETTINGS_FILE))
endif

RAYLIB ?= true
EMULATOR ?= $(ROOT)/emulator/riscv/cm2-riscv-emulator

#source files
CSRCS = $(FS_SELECT) \
		$(DEV_SELECT) \
	   $(wildcard $(ROOT)/kernel/*.c) \
	   $(wildcard $(ROOT)/arch/$(ARCH)/*.c) \
	   $(wildcard $(ROOT)/lib/*.c)

ASRCS = $(wildcard $(ROOT)/arch/$(ARCH)/*.S)

#include options
INCL ?= -I$(ROOT)/include

#linker file
LNKF = $(ROOT)/arch/$(ARCH)/linker.ld

# link path options
LNKP ?=

MN_FILE ?= main.elf

# IF debugging stack faliures:
# make DEBUG=true

DEBUG ?= false

CFLAGS = $(ARCH_CFLAGS) -ffreestanding -Wall -Wextra -Wno-unused-parameter  $(INCL)
ASFLAGS = $(CFLAGS)
LDFLAGS = $(ARCH_LDFLAGS) -nostdlib -nostartfiles -static


ifeq ($(DEBUG), true)
	CFLAGS += -D__DEBUG__ -g -fverbose-asm
endif

CFLAGS += -Os
LDFLAGS += -Os
ASFLAGS += -fno-lto

ifeq ($(CONFIG_LTO), y)
CFLAGS += -flto
LDFLAGS += -flto
endif

OBJS = $(CSRCS:%.c=%.o) $(ASRCS:%.S=%.o)

CC = $(TOOLCHAIN)-gcc 
OBJCOPY = $(TOOLCHAIN)-objcopy
READELF = $(TOOLCHAIN)-readelf

.PHONY: userspace settings install

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(ASFLAGS) -c $< -o $@


$(MN_FILE): $(OBJS) settings
	$(CC) $(LDFLAGS) -T $(LNKF) $(OBJS) -o $@

image: $(MN_FILE)
	$(OBJCOPY) -O binary $(MN_FILE) image.bin
ifeq ($(CONFIG_ARCH_TAURUS), y)
	/bin/env python3 $(ROOT)/scripts/$(ARCH)_encoder.py image.bin
endif

STAGING = $(ROOT)/staging
USERSPACE = $(ROOT)/userspace

userspace:
	mkdir -p $(STAGING)
	mkdir -p $(STAGING)/dev
	mkdir -p $(STAGING)/bin
	mkdir -p $(STAGING)/home
	cp $(USERSPACE)/README $(STAGING)/README
	$(MAKE) -C $(USERSPACE) ROOT=$(USERSPACE) STAGING=$(STAGING) TOOLCHAIN=$(TOOLCHAIN) KERNEL_HEADERS=$(ROOT)/include/uapi all
	/bin/env python3 $(ROOT)/scripts/fat8.mkfs.py $(STAGING)

run: image.bin
	$(MAKE) -C $(EMULATOR) run ROOT="$(EMULATOR)" OUTPUT_ARGS="taurus $(ROOT)/image.bin $(EMULATOR)/emulator-tilesheet/minesweeper.bmp $(ROOT)/fat8.img $(ROOT)/emulator-sprites"

tools:
	$(MAKE) -C $(EMULATOR) all ROOT="$(EMULATOR)" RAYLIB="$(RAYLIB)" OPTIMIZE=true

size:
	$(READELF) -S $(MN_FILE)

dump:
	$(TOOLCHAIN)-objdump -d -M no-aliases main.elf >> dump.s.dump
	$(TOOLCHAIN)-objdump -S -d -M no-aliases main.elf >> verbose_dump.s.dump

clean:
	rm -rf $(STAGING)
	rm -f $(OBJS) $(MN_FILE) image.bin fat8.img
	rm -rf dump.s.dump
	rm -rf verbose_dump.s.dump
	$(MAKE) -C $(USERSPACE) ROOT=$(USERSPACE) STAGING=$(STAGING) TOOLCHAIN=$(TOOLCHAIN) KERNEL_HEADERS=$(ROOT)/include/uapi clean

all: image size

rebuild: clean all

install:
	pyocd list
	pyocd load --format elf $(MN_FILE)

menuconfig:
	kconfig-mconf ./Kconfig

