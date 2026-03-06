#include <stdint.h>
#include <stddef.h>

uint32_t strlen(const char* str);

uint32_t strnlen(const char* str, uint32_t max_size);

int strncmp(const char * s1, const char * s2, uint32_t n);

int strncpy(char* dest, char* src, uint32_t size);

//DANGER: enforces \0 in the buffer by writing '\0' to size + 1
int strlcpy(char* dest, char* src, uint32_t size);

void memset(void* dest, uint8_t value, uint32_t size);

void* memcpy(void* dest, void* source, size_t size);


