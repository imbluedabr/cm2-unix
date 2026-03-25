#include <lib/stdlib.h>
#include <stddef.h>
#include <stdbool.h>

#define MAX_ALLOCATIONS 8

uint8_t* heap_start;

static int allocations_size = 1;
static struct {
    void *block;
    size_t size;
    bool occupied;
} allocations[MAX_ALLOCATIONS];

void heap_init(uint8_t* base)
{
    heap_start = base;
    allocations[0].block = base;
    allocations[0].size = 0;
    allocations[0].occupied = false;
}

void *malloc(size_t size)
{
    if (size == 0) {
        return NULL;
    }
    if (allocations_size + 1 == MAX_ALLOCATIONS) {
        return NULL;
    }

    for (int i = 0; i < allocations_size; i++) {
        if (allocations[i].occupied == false && (allocations[i].size == 0 || size <= allocations[i].size)) {
            allocations[i].size = size;
            allocations[i].occupied = true;
            return allocations[i].block;
        }
    }
    allocations[allocations_size].block = allocations[allocations_size - 1].block + allocations[allocations_size - 1].size + 4;
    allocations[allocations_size].size = size;
    allocations[allocations_size].occupied = true;
    return (allocations[allocations_size++].block = (void *)((uintptr_t)allocations[allocations_size].block & 0xfffffffc));
}

void free(void *ptr)
{
    for (int i = 0; i < allocations_size; i++) {
        if (allocations[i].block == ptr && allocations[i].occupied) {
            allocations[i].occupied = false;
        }
    }
}

void *realloc(void *ptr, size_t size)
{
    void *alloc = malloc(size);
    if (!alloc) {
        return alloc;
    }
    memcpy(alloc, ptr, size);
    free(ptr);
    return alloc;
}
