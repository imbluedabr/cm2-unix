#include <kernel/device.h>
#include <kernel/tty.h>
#include <kernel/block.h>
#include <kernel/majors.h>

const char msg1[] = "CM2-UNIX V0.0.1\n";
const char msg2[] = "reg: disk0 at address 0xFFC3\n";
dev_t tty0_devno;
struct device* tty0;

dev_t disk0_devno;

struct device_request msg;

enum {
    REG_TTY,
    REG_DSK,
    END
} init_state;

void init_update() {
    switch(init_state) {
        case REG_TTY:
            tty0 = device_create(&tty0_devno, TTY_MAJOR, (void*) 0xFFF1);
            msg.buffer = msg1;
            msg.count = sizeof(msg1)-1;
            msg.operation = DEVICE_OP_WR;
            device_queue_action(tty0, &msg);
            init_state = REG_DSK;
            break;
        case REG_DSK:
            if (msg.state != DEVICE_STATE_FINISHED) break;
            device_create(&disk0_devno, GEN_DISK_MAJOR, (void*) 0xFFC3);
            msg.buffer = msg2;
            msg.count = sizeof(msg2)-1;
            device_queue_action(tty0, &msg);
            init_state = END;
            break;
        case END:
            break;
    }
}

void main(void) {
    tty_init();
    gen_disk_init();
    
    
    while (1) {
        init_update();

        device_update();
    }
}



