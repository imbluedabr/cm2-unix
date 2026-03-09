#pragma once
#include <kernel/device.h>

extern struct device* console;
void kputc(char c);
void kputs(const char *str);
void kprintf(const char *fmt, ...);
