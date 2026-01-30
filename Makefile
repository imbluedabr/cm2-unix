
ARCH ?= riscv

TOOLCHAIN ?= riscv64-unknown-elf

ROOT ?= .

#source files
CSRCS = $(wildcard $(ROOT)/fs/*.c) \
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

CFLAGS ?= -march=rv32i -mabi=ilp32 -fno-pic -fno-pie -ffreestanding -Os -Wall -Wextra -Wno-unused-parameter $(INCL)

LDFLAGS ?= -nostdlib -nostartfiles -static -march=rv32i -mabi=ilp32 -Os

OBJS = $(CSRCS:%.c=%.o) $(ASRCS:%.S=%.o)

CC = $(TOOLCHAIN)-gcc 
OBJCOPY = $(TOOLCHAIN)-objcopy
READELF = $(TOOLCHAIN)-readelf

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

$(MN_FILE): $(OBJS)
	$(CC) $(LDFLAGS) -T $(LNKF) $(OBJS) -o $@

image: $(MN_FILE)
	$(OBJCOPY) -O binary $(MN_FILE) image.bin

size:
	$(READELF) -S $(MN_FILE)

clean:
	rm $(OBJS) $(MN_FILE) image.bin

