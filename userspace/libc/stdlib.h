#pragma once
#include <stdint.h>
#include <stddef.h>
#include <proc.h>
#include <vfs.h>

uint32_t strlen(const char* str);

uint32_t strnlen(const char* str, uint32_t max_size);

int strncmp( const char * s1, const char * s2, uint32_t n );

int strncpy(char* dest, char* src, uint32_t size);

void strlcpy(char* dest, char* src, uint32_t size);

void memset(void* dest, uint8_t value, uint32_t size);

void* memcpy(void* dest, void* source, size_t size);

void int_to_hex(char* buff, uint16_t val, uint8_t numc);

int open(const char* path, uint32_t flags);

int read(int fd, void* buffer, int count);

int write(int fd, const void* buffer, int count);

int readdir(int dirfd, struct stat* buffer, int count);

int ioctl(int fd, int cmd, void* arg);

void exit(int exit_code);

int exec(const char* path, const char** argv, int* fileno_vec);

int wait(pid_t upid);

int sysctl(int cmd, void* buff, int count);

void yield();


