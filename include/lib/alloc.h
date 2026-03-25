#pragma once
#include <stdint.h>
#include <stddef.h>

void heap_init(uint8_t* base);
void *malloc(size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);
