#include <kernel/device.h>
#include <kernel/tty.h>
#include <kernel/block.h>
#include <kernel/majors.h>
#include <kernel/proc.h>
#include <kernel/syscall.h>

dev_t tty0_devno;
struct device* tty0;
dev_t disk0_devno;

uint8_t init_thread_stack[64];

void init_thread() {
    char test[] = "CM2-UNIX V0.1.0\nBooting...\n";
    syscall(0, tty0_devno, (uint32_t) &test, sizeof(test) - 1);

    while (1) {};
}

void main() {
    *((uint8_t*) 0xFFF6) = 0;
    
    //initialize functions
    device_init();
    tty_init();
    gen_disk_init();
    proc_init();

    tty0 = device_create(&tty0_devno, TTY_MAJOR, (void*) 0xFFF1);
    
    proc_create((uint32_t) &init_thread, (uint32_t) &init_thread_stack[63]);
    
    //bootstrap the scheduler by getting the first process to run
    current_process = proc_dequeue();
    exit_kernel(); //jump into the current process
}


