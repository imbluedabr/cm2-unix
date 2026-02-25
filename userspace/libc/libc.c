#include "stdlib.h"
#include "stdio.h"

#include <syscalls.h>
#include <stdarg.h>

uint8_t STDOUT;

uint32_t strlen(const char* str)
{
    int size = 0;
    while (*str++ != '\0') {
        size++;
    }
    return size;
}

uint32_t strnlen(const char* str, uint32_t max_size)
{
    uint32_t size = 0;
    while (*str++ != '\0') {
        if (size > max_size) {
            break;
        }
        size++;
    }
    return size;
}


int strncmp( const char * s1, const char * s2, uint32_t n )
{
    while ( n && *s1 && ( *s1 == *s2 ) )
    {
        ++s1;
        ++s2;
        --n;
    }
    if ( n == 0 )
    {
        return 0;
    }
    else
    {
        return ( *(unsigned char *)s1 - *(unsigned char *)s2 );
    }
}

int strncpy(char* dest, char* src, uint32_t size)
{
    int i = 0;
    while((*src != '\0') && (size-- > 0)) {
        *(dest++) = *(src++);
        i++;
    }
    return i;
}

void strlcpy(char* dest, char* src, uint32_t size)
{
    while((*src != '\0') && (size-- > 0)) {
        *(dest++) = *(src++);
    }
    *dest = '\0';
}


void memset(void* dest, uint8_t value, uint32_t size)
{
    while(size-- > 0) {
        ((uint8_t*) dest)[size] = value;
    }
}


[[gnu::used]] void* memcpy(void* dest, void* source, size_t size)
{
    while(size-- > 0) {
        ((uint8_t*) dest)[size] = ((uint8_t*) source)[size];
    }
    return dest;
}



void int_to_hex(char* buff, uint16_t val, uint8_t numc)
{
    static const char lookup[] = "0123456789abcdef";
    for (int i = 0; i < numc; i++) {
        buff[numc - i - 1] = lookup[val & 0xf];
        val >>= 4;
    }
}

const char *u32_to_hex(uint32_t value) {
    static const char lookup[] = "0123456789abcdef";
    static char buf[9];
    for (int i = 0; i < 8; i++) {
        buf[7 - i] = lookup[value & 0xf];
        value >>= 4;
    }
    buf[8] = '\0';
    return buf;
}


int open(const char* path, uint32_t flags)
{
    return SYSCALL(SYS_OPEN, (uint32_t) path, flags, 0);
}

int read(int fd, void* buffer, int count)
{
    return SYSCALL(SYS_READ, fd, (uint32_t) buffer, count);
}

int write(int fd, const void* buffer, int count)
{
    return SYSCALL(SYS_WRITE, fd, (uint32_t) buffer, count);
}

int readdir(int dirfd, struct stat* buffer, int count)
{
    return SYSCALL(SYS_READDIR, dirfd, (uint32_t) buffer, count);
}

int ioctl(int fd, int cmd, void* arg)
{
    return SYSCALL(SYS_IOCTL, fd, cmd, (uint32_t) arg);
}

int getcwd(char* buff, int size)
{
    return SYSCALL(SYS_GETCWD, (uint32_t) buff, (uint32_t) size, 0);
}

int chdir(const char* path)
{
    return SYSCALL(SYS_CHDIR, (uint32_t) path, 0, 0);
}


void exit(int exit_code)
{
    SYSCALL(SYS_EXIT, exit_code, 0, 0);
}

int wait(pid_t upid)
{
    return SYSCALL(SYS_WAIT, upid, 0, 0);
}

int exec(const char* path, const char** argv, int* fileno_vec)
{
    return SYSCALL(SYS_EXEC, (uint32_t) path, (uint32_t) argv, (uint32_t) fileno_vec);
}

int sysctl(int cmd, void* buff, int count)
{
    return SYSCALL(SYS_SYSCTL, cmd, (uint32_t) buff, count);
}

void yield()
{
    SYSCALL(SYS_YIELD, 0, 0, 0);
}

void putc(char c)
{
    write(STDOUT, &c, 1);
}

void puts(const char *str)
{
    write(STDOUT, str, strlen(str));
}

void printf(const char *fmt, ...) 
{
    va_list params;
    va_start(params, fmt);

    while (*fmt != '\0') {
        if (*fmt != '%') putc(*fmt);
        else {
            fmt++;

            if (*fmt == 'c') {
                char ch = va_arg(params, int);
                putc(ch);
            }
            else if (*fmt == 's') {
                char *s = va_arg(params, char *);
                puts(s);
            }
            else if (*fmt == 'x') {
                int num = va_arg(params, int);
                char *hex = u32_to_hex(num);
                puts(hex);
            }
        }
        fmt++;
    }
}


