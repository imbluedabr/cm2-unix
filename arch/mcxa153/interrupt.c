#include <arch/mcxa153/interrupt.h>
#include <arch/mcxa153/fault.h>
#include <arch/mcxa153/MCXA153.h>
#include <lib/stdlib.h>
#include <stddef.h>


[[gnu::aligned(512)]] uint32_t vector_table[VECTOR_TABLE_SIZE];

struct device* interrupt_registry[VECTOR_TABLE_SIZE];

void interrupt_init()
{
    memcpy(vector_table, __Vectors, 16*4); //copy the core interrupts to the new table
    //memset(vector_table, 0, sizeof(vector_table));
    memset(interrupt_registry, 0, sizeof(interrupt_registry));
    __DSB();

    SCB->VTOR = (uint32_t) &vector_table;
    
    __DSB();
    __ISB();
}

int register_interrupt(int ivec, struct device* dev, void (*handler)(void))
{
    if ((16 + ivec) >= VECTOR_TABLE_SIZE) {
        return -1;
    }
    if (vector_table[16 + ivec]) {
        return -1; //interrupt vector already assigned
    }
    vector_table[16 + ivec] = (uint32_t) handler;
    __DSB();
    __ISB();

    interrupt_registry[16 + ivec] = dev;
    return 0;
}

void set_interrupt_priority(int ivec, int priority)
{
    NVIC_SetPriority(ivec, priority);
}

int get_current_interrupt()
{
    return __get_IPSR() - 16;
}

struct device* get_current_device() {
    return interrupt_registry[__get_IPSR()];
}


