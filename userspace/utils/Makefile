
# this is set by the root makefile
STAGING ?=
TOOLCHAIN ?=
KERNEL_HEADERS ?= 
LIBC ?=

CC = $(TOOLCHAIN)-gcc

include $(LIBC)/base.mk

CFLAGS = $(BASE_CFLAGS) -Os -flto -I$(KERNEL_HEADERS) -I$(LIBC)
LDFLAGS = $(BASE_LDFLAGS) -Os -flto -T$(LIBC)/linker.ld $(LIBC)/*.o

all: clean ls cat ps install


ls.o: ls.c
	$(CC) $(CFLAGS) -c ls.c -o ls.o

ls: ls.o
	$(CC) $(LDFLAGS) ls.o -o ls.elf
	$(TOOLCHAIN)-objcopy -O binary ls.elf ls.bin

cat.o: cat.c
	$(CC) $(CFLAGS) -c cat.c -o cat.o

cat: cat.o
	$(CC) $(LDFLAGS) cat.o -o cat.elf
	$(TOOLCHAIN)-objcopy -O binary cat.elf cat.bin

ps.o: ps.c
	$(CC) $(CFLAGS) -c ps.c -o ps.o

ps: ps.o
	$(CC) $(LDFLAGS) ps.o -o ps.elf
	$(TOOLCHAIN)-objcopy -O binary ps.elf ps.bin

install:
	cp ls.bin $(STAGING)/bin/ls
	cp cat.bin $(STAGING)/bin/cat
	cp ps.bin $(STAGING)/bin/ps

clean:
	rm -rf ls.o ls.elf cat.o cat.elf ps.o ps.elf



