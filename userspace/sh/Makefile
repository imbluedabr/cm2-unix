
# this is set by the root makefile
STAGING ?=
TOOLCHAIN ?=
KERNEL_HEADERS ?= 
LIBC ?=

CC = $(TOOLCHAIN)-gcc

include $(LIBC)/base.mk

CFLAGS = $(BASE_CFLAGS) -Os -flto -I$(KERNEL_HEADERS) -I$(LIBC)
LDFLAGS = $(BASE_LDFLAGS) -Os -flto -T$(LIBC)/linker.ld $(LIBC)/*.o


all: clean sh install

sh.o: sh.c
	$(CC) $(CFLAGS) -c sh.c -o sh.o

sh: sh.o
	$(CC) $(LDFLAGS) sh.o -o sh.elf
	$(TOOLCHAIN)-objcopy -O binary sh.elf sh.bin

install:
	cp sh.bin $(STAGING)/bin/sh

clean:
	rm -rf sh.o sh.elf



