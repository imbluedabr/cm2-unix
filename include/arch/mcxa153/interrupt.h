#include <stdint.h>

//initialize the ram interrupt vector table
void interrupt_init();

int register_interrupt(uint8_t ivec, void (*handler)(void));

void set_interrupt_priority(uint8_t ivec, uint8_t priority);

