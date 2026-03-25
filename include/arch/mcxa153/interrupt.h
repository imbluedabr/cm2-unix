#include <stdint.h>
#include <kernel/device.h>

#define VECTOR_TABLE_SIZE 96
extern uint32_t vector_table[VECTOR_TABLE_SIZE];
extern uint32_t __Vectors[VECTOR_TABLE_SIZE];


//initialize the ram interrupt vector table
void interrupt_init();

int register_interrupt(int ivec, struct device* dev, void (*handler)(void));

void set_interrupt_priority(int ivec, int priority);

int get_current_interrupt();

struct device* get_current_device();

