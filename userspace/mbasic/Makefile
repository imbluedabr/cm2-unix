
# this is set by the root makefile
STAGING ?=
TOOLCHAIN ?=
KERNEL_HEADERS ?= 
LIBC ?=

CC = $(TOOLCHAIN)-gcc

include $(LIBC)/base.mk

CFLAGS = $(BASE_CFLAGS) -Os -flto -I$(KERNEL_HEADERS) -I$(LIBC)
LDFLAGS = $(BASE_LDFLAGS) -Os -flto -T$(LIBC)/linker.ld $(LIBC)/*.o


all: clean mbasic test install

basic.o: basic.c
	$(CC) $(CFLAGS) -c basic.c -o basic.o

mbasic: basic.o
	$(CC) $(LDFLAGS) basic.o -o mbasic.elf
	$(TOOLCHAIN)-objcopy -O binary mbasic.elf mbasic.bin

test.o: test.c
	$(CC) $(CFLAGS) -c test.c -o test.o
test: test.o
	$(CC) $(LDFLAGS) test.o -o test.elf
	$(TOOLCHAIN)-objcopy -O binary test.elf test.bin

install:
	cp mbasic.bin $(STAGING)/bin/mbasic
	cp test.bin $(STAGING)/bin/test

clean:
	rm -rf mbasic.o mbasic.elf






