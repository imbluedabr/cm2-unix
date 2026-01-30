#include <kernel/device.h>
#include <kernel/tty.h>
#include <kernel/majors.h>

const char init_msg[] = "CM2-UNIX V0.0.1-PRE\n";

void main(void) {
    tty_init();
    
    dev_t tty0_devno;
    struct device* tty0 = device_create(&tty0_devno, TTY_MAJOR, (void*) 0xFFF1); //0xFFF1 is the base address of tty0
    
    tty0->ops->write(tty0, init_msg, sizeof(init_msg), 0);
}



