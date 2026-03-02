#include <lib/stdlib.h>
#include <lib/hex.h>
#include <kernel/proc.h>
#include <lib/kprint.h>

#include <stdint.h>

void panic(const char *s) {
    uint32_t sp;
    __asm__ volatile (
        "mv %0, sp"
        : "=r" (sp)
    );
    
    kprintf("KERNEL PANIC: \n%s\n", s ? s : "No reasson provided.");
    kprintf("SP: %x\n", sp);
    kprintf("PID: %x\n", current_process->pid);
    kprintf("SYSCALL: %x\n", current_process->syscall_operation);
    kprintf("RA: %x\n", current_process->return_address);
    
    while (1);
}
