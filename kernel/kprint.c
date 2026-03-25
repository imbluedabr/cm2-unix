#include <lib/kprint.h>
#include <stdint.h>
#include <stdarg.h>
#include <lib/hex.h>


struct device* console;

void kputc(char c)
{
    while(console->ops->writeb(console, c) == -1);
}

void kputs(const char *str)
{
    while (*str != '\0')
    {
        while(console->ops->writeb(console, *str) == -1);
        str++;
    }
}

void kprintf(const char *fmt, ...) 
{
    va_list params;
    va_start(params, fmt);

    while (*fmt != '\0') {
        if (*fmt != '%') kputc(*fmt);
        else {
            fmt++;

            if (*fmt == 'c') {
                char ch = va_arg(params, int);
                kputc(ch);
            }
            else if (*fmt == 's') {
                char *s = va_arg(params, char *);
                kputs(s);
            }
            else if (*fmt == 'x') {
                int num = va_arg(params, int);
                const char *hex = u32_to_hex(num);
                kputs(hex);
            }
        }
        fmt++;
    }
}
