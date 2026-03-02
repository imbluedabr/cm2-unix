#include <stddef.h>
#include <stdbool.h>
#include <kernel/device.h>
#include <uapi/majors.h>
#include <drivers/tty.h>
#include <arch/riscv/memorymap.h>


//the hardware interface implementation
struct tty_hardware_interface {
    char input_character;
    uint8_t input_ready;
    uint8_t clear;
    uint8_t write;
    char character;
    uint8_t cursor_location;
};

//this is the table that stores the tty instances
#define MAX_TTY_COUNT 2
#define MAX_TTY_MSK (MAX_TTY_COUNT-1)
struct tty_device tty_table[MAX_TTY_COUNT];

//tty operations
const struct device_ops tty_ops = {
    .ioctl = &tty_ioctl,
    .update = &tty_update
};

void tty_init() {
    struct device_driver* tty_driver = &driver_registry[TTY_MAJOR];
    tty_driver->create = &tty_create;
    tty_driver->destroy = &tty_destroy;
    tty_driver->lookup = &tty_lookup;
    tty_driver->update = &tty_global_update;
    tty_driver->name = "generic tty";
}

//create a tty instance
struct device* tty_create(int8_t* minor, const void* args)
{
    for (int8_t i = 0; i < MAX_TTY_COUNT; i++) {
        struct tty_device* tty = &tty_table[i];
        if (tty->base.ops == NULL) {
            tty->base.ops = (struct device_ops*) &tty_ops;
            tty->tty = (struct tty_hardware_interface*) args;
            tty->base.count = 0; //init queue
            tty->base.head = 0;
            tty->base.tail = 0;
            *minor = i;
            return &tty->base;
        }
    }
    *minor = -1; //TODO: add error code for this
    return NULL;
}

//destroy a tty instance
int tty_destroy(uint8_t minor)
{
    struct tty_device* tty = &tty_table[minor & MAX_TTY_MSK];
    tty->base.ops = NULL;
    tty->tty = NULL;
    return 0;
}

//lookup a tty instance
struct device* tty_lookup(uint8_t minor)
{
    struct tty_device* tty = &tty_table[minor & MAX_TTY_MSK];
    if (tty->base.ops == NULL) {
        return NULL;
    }
    return &tty->base;
}

void tty_global_update() {
    for (int i = 0; i < MAX_TTY_COUNT; i++) {
        struct tty_device* tty = &tty_table[i];
        if (tty->base.ops != NULL) {
            tty_update(&tty->base);
        }
    }
}

int tty_ioctl(struct device* dev, int cmd, void* arg) {
    struct tty_device* tty = (struct tty_device*) dev;
    volatile struct tty_hardware_interface* tty_interface = tty->tty;
    int return_code = 0;

    if (tty->base.ops == NULL) {
        return 0;
    }
    
    switch(cmd) {
        case TTY_IOCTL_CLEAR:
            tty_interface->clear = 1;
            tty_interface->cursor_location = 0;
            break;
        case TTY_IOCTL_SETCURSOR:
            tty_interface->cursor_location = *((uint8_t*) arg);
            break;
        case TTY_IOCTL_SETMODE:
            tty->mode = *((uint16_t*) arg);
            break;
        case TTY_IOCTL_GETMODE:
            return_code = tty->mode;
            break;
    }

    return return_code;
}

static inline uint8_t tty_write(
        struct tty_device* tty,
        struct device_request* current_req,
        volatile struct tty_hardware_interface* tty_interface
        )
{
    uint32_t i = tty->current_bytes_copied;
    char c = ((char*) current_req->buffer)[i];

    if (c == '\n' || tty_interface->cursor_location == 255) {
        tty_interface->cursor_location = (tty_interface->cursor_location + 32) & 0b11100000;
    } else {
        tty_interface->character = c;
        tty_interface->write = 1;
        tty_interface->cursor_location++;
    }

    tty->current_bytes_copied = ++i;

    if (i == current_req->count) {
        return 1;
    }
    return 0;
}

static inline uint8_t tty_read(
        struct tty_device* tty,
        struct device_request* current_req,
        volatile struct tty_hardware_interface* tty_interface
        )
{
    
    if (tty_interface->input_ready) {
        char c = tty_interface->input_character;
        uint32_t i = tty->current_bytes_copied;

        if (c == '\r' || c == '\n') {
            tty_interface->cursor_location = (tty_interface->cursor_location + 32) & 0b11100000;
            return 1;
        } else if (c == 127 || c == 8) {
        	if (!(i >= 1)) return 0;
            ((char*) current_req->buffer)[--i] = 0;
            tty->current_bytes_copied = i;
            tty_interface->character = ' ';
            tty_interface->cursor_location--;
            tty_interface->write = 1;
            return 0;
        }

        //echo character
        tty_interface->character = c;
        tty_interface->write = 1;
        tty_interface->cursor_location++;
        
        ((char*) current_req->buffer)[i++] = c;
        tty->current_bytes_copied = i;
        
        if (i == current_req->count) {
            return 1;
        }
    }

    return 0;
}


void tty_update(struct device* dev)
{
    struct tty_device* tty = (struct tty_device*) dev;
    volatile struct tty_hardware_interface* tty_interface = tty->tty;
    struct device_request* current_req = tty->current_req;

    if (current_req == NULL) {
        current_req = device_queue_pop(dev);
        if (current_req == NULL)
            return;

        tty->current_req = current_req;
        tty->current_bytes_copied = 0;
    }
    
    uint8_t exit;
    if (current_req->operation == DEVICE_OP_WR) {
        exit = tty_write(tty, current_req, tty_interface);
    } else {
        exit = tty_read(tty, current_req, tty_interface);
    }

    if (exit) {
        current_req->state = DEVICE_STATE_FINISHED;
        current_req->count = tty->current_bytes_copied;
        tty->current_req = NULL;
    }

}
