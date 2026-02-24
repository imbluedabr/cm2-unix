
# this is set by the root makefile
STAGING ?=
TOOLCHAIN ?=
#kernel headers
KERNEL_HEADERS ?= 

CC = $(TOOLCHAIN)-gcc

include base.mk

CFLAGS = $(BASE_CFLAGS) -Os -flto -I$(KERNEL_HEADERS)

SRCS = $(wildcard *.c)
OBJS = $(SRCS:%.c=%.o) start.o


%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all libc clean

all: clean libc

libc: $(OBJS)

clean:
	rm -rf $(OBJS)
