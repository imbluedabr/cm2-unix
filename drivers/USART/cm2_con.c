#include <stddef.h>
#include <stdbool.h>
#include "cm2_con.h"
#include <drivers/usart.h>
#include <lib/kprint.h>
#include <arch/riscv/memorymap.h>

//the hardware interface implementation
struct cm2con_hardware_interface {
    char input_character;
    uint8_t input_ready;
    uint8_t clear;
    uint8_t write;
    char character;
    uint8_t cursor_location;
};

int cm2con_ioctl(struct device* dev, int cmd, void* arg) {
    struct usart_device* cm2con = (struct usart_device*) dev;
    //volatile struct cm2con_hardware_interface* cm2con_interface = cm2con->usart_base;
    int return_code = 0;

    if (!cm2con->base.ops) {
        return 0;
    }
    
    switch(cmd) {
        default:
            return_code = -1; //invalid ioctl
    }

    return return_code;
}


int cm2con_readb(struct device* dev)
{
    struct usart_device* cm2con = (struct usart_device*) dev;
    volatile struct cm2con_hardware_interface* cm2con_interface = cm2con->usart_base;
    
    if (cm2con_interface->input_ready) {
        return cm2con_interface->input_character;
    }
    return -1;
}

int cm2con_writeb(struct device* dev, uint8_t val)
{
    struct usart_device* cm2con = (struct usart_device*) dev;
    volatile struct cm2con_hardware_interface* cm2con_interface = cm2con->usart_base;
    
    if (cm2con->mode & CM2CON_ESC_STATE && val == '[') {
        cm2con->mode &= ~CM2CON_ESC_STATE;
        cm2con->mode |= CM2CON_CSI_STATE;
        return 0;
    }

    if (cm2con->mode & CM2CON_CSI_STATE && val == 'J') {
        cm2con_interface->clear = 1;
        cm2con_interface->cursor_location = 0;
        cm2con->mode &= ~CM2CON_CSI_STATE;
        return 0;
    }


    if (val == '\n' || cm2con_interface->cursor_location == 255) {
        cm2con_interface->cursor_location = (cm2con_interface->cursor_location + 32) & 0b11100000;
    } else if (val == '\b') {
        cm2con_interface->cursor_location--;
    } else if (val == '\e') {
        cm2con->mode |= CM2CON_ESC_STATE;
    } else {
        cm2con_interface->character = val;
        cm2con_interface->write = 1;
        cm2con_interface->cursor_location++;
    }
    return 0;
}

static inline uint8_t cm2con_write(
        struct usart_device* cm2con,
        struct device_request* current_req,
        volatile struct cm2con_hardware_interface* cm2con_interface
        )
{
    uint16_t i = cm2con->bytes_copied;
    char c = ((char*) current_req->buffer)[i];

    cm2con_writeb(&cm2con->base, c);

    cm2con->bytes_copied = ++i;

    if (i == current_req->count) {
        return 1;
    }
    return 0;
}


void cm2con_update(struct device* dev)
{
    struct usart_device* cm2con = (struct usart_device*) dev;
    volatile struct cm2con_hardware_interface* cm2con_interface = cm2con->usart_base;
    struct device_request* current_req = cm2con->current_req;

    if (current_req == NULL) {
        current_req = device_queue_pop(dev);
        if (current_req == NULL)
            return;

        cm2con->current_req = current_req;
        cm2con->bytes_copied = 0;
    }
    
    uint8_t exit;
    if (current_req->operation == DEVICE_OP_WR) {
        exit = cm2con_write(cm2con, current_req, cm2con_interface);
    } else {
        exit = 1;
    }

    if (exit) {
        current_req->state = DEVICE_STATE_FINISHED;
        current_req->count = cm2con->bytes_copied;
        cm2con->current_req = NULL;
    }

}




