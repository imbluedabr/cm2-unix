#include <stdint.h>

uint32_t strnlen(const char* str, uint32_t max_size);

int strncmp(const char * s1, const char * s2, uint32_t n);

void strncpy(char* dest, char* src, uint32_t size);

void memset(void* dest, uint32_t size, uint8_t value);

void memcpy(void* dest, void* source, uint32_t size);


