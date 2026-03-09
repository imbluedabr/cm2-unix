#include <arch/mcxa153/interrupt.h>
#include <arch/mcxa153/fault.h>
#include <arch/mcxa153/MCXA153.h>
#include <stddef.h>

#define VECTOR_TABLE_SIZE 128
[[gnu::aligned(128)]] void (*vector_table[VECTOR_TABLE_SIZE])(void);

void interrupt_init()
{
    vector_table[0] = reset_handler;
    vector_table[15 + NonMaskableInt_IRQn] = NMI_handler;
    vector_table[15 + HardFault_IRQn] = hardfault_handler;
    vector_table[15 + MemoryManagement_IRQn] = MPUfault_handler;
    vector_table[15 + BusFault_IRQn] = busfault_handler;
    vector_table[15 + UsageFault_IRQn] = usagefault_handler;
    vector_table[15 + SecureFault_IRQn] = securefault_handler;
    SCB->VTOR = (uint32_t) &vector_table;
}

int register_interrupt(uint8_t ivec, void (*handler)(void))
{
    if ((15 + ivec) > VECTOR_TABLE_SIZE) {
        return -1;
    }
    if (vector_table[15 + ivec]) {
        return -1; //interrupt vector already assigned
    }
    vector_table[15 + ivec] = handler;
    return 0;
}

void set_interrupt_priority(uint8_t ivec, uint8_t priority)
{
    NVIC_SetPriority(ivec, priority);
}



