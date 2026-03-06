#include <drivers/tty.h>
#include <uapi/majors.h>
#include <uapi/tty.h>
#include <lib/kprint.h>
#include <stddef.h>

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
    tty_driver->name = "tty driver";
}

//create a tty instance
struct device* tty_create(int8_t* minor, const void* args)
{
    const struct tty_desc* desc = args;
    for (int8_t i = 0; i < MAX_TTY_COUNT; i++) {
        struct tty_device* tty = &tty_table[i];
        if (tty->base.ops == NULL) {
            tty->base.ops = (struct device_ops*) &tty_ops;
            tty->base.count = 0; //init queue
            tty->base.head = 0;
            tty->base.tail = 0;
            tty->read_dev = device_lookup(desc->read_dev);
            tty->write_dev = device_lookup(desc->write_dev);
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
    int return_code = 0;

    if (tty->base.ops == NULL) {
        return 0;
    }
    
    switch(cmd) {
        case TTY_IOCTL_SETMODE:
            tty->mode = *((uint16_t*) arg);
            break;
        case TTY_IOCTL_GETMODE:
            return_code = tty->mode;
            break;
    }

    return return_code;
}


static void tty_read(struct tty_device* tty, struct device_request* req)
{
    struct device* read_dev = tty->read_dev;
    if (!read_dev || !read_dev->ops->readb) {
        goto end;
    }

    struct device* write_dev = tty->write_dev;
    
    int c = read_dev->ops->readb(read_dev);
    if (c >= 0) {
        if (c == 0x7F) { //quick fix since sometimes backspace is DEL instead of '\b'
            c = '\b';
        }
        if (c == '\b') {
            if (tty->bytes_copied == 0) {
                return;
            }
            write_dev->ops->writeb(write_dev, '\b');
            write_dev->ops->writeb(write_dev, ' ');
            tty->bytes_copied--;
        }
        write_dev->ops->writeb(write_dev, c);

        if (c == '\n') {
            goto end;
        } else if (c == '\b') {
            return;
        }
        
        
        int i = tty->bytes_copied;
        ((uint8_t*) req->buffer)[i] = c;
        tty->bytes_copied = ++i;
        if (i > req->count) {
            goto end;
        }
    }
    return;
end:
    req->state = DEVICE_STATE_FINISHED;
    req->count = tty->bytes_copied;
    tty->current_req = NULL;
    return;
}

void tty_update(struct device* dev)
{
    struct tty_device* tty = (struct tty_device*) dev;
    struct device_request* current_req = tty->current_req;

    if (current_req == NULL) {
        current_req = device_queue_pop(dev);
        if (current_req == NULL)
            return;

        tty->current_req = current_req;
        tty->bytes_copied = 0;
    }
    
    if (current_req->operation == DEVICE_OP_WR) {
        
        //pass the request on to the write device
        if(device_queue_action(tty->write_dev, current_req) == 0) {//if we succesfully passed on the request we set the current request to null
            tty->current_req = NULL;
        }

    } else {
        tty_read(tty, current_req);
    }

}














