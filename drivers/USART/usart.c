#include <stddef.h>
#include <stdbool.h>
#include <kernel/device.h>
#include <uapi/majors.h>
#include <drivers/usart.h>
#include "cm2_con.h"

//this is the table that stores the tty instances
#define MAX_USART_COUNT 2
#define MAX_USART_MSK (MAX_USART_COUNT-1)
struct usart_device usart_table[MAX_USART_COUNT];

const struct device_ops usart_backends[] = {
#ifdef USART_DRIVER_CM2CON
    {
        .readb = &cm2con_readb,
        .writeb = &cm2con_writeb,
        .ioctl = &cm2con_ioctl,
        .update = &cm2con_update
    }
#endif
};

void usart_init() {
    struct device_driver* usart_driver = &driver_registry[USART_MAJOR];
    usart_driver->create = &usart_create;
    usart_driver->destroy = &usart_destroy;
    usart_driver->lookup = &usart_lookup;
    usart_driver->update = &usart_global_update;
    usart_driver->name = "usart driver";
}

//create a tty instance
struct device* usart_create(int8_t* minor, const void* args)
{
    const struct usart_desc* desc = args;
    for (int8_t i = 0; i < MAX_USART_COUNT; i++) {
        struct usart_device* usart = &usart_table[i];
        if (!usart->base.ops) {
            usart->base.ops = (struct device_ops*) &usart_backends[desc->device_id];
            usart->base.count = 0; //init queue
            usart->base.head = 0;
            usart->base.tail = 0;
            usart->usart_base = desc->base;
            *minor = i;
            return &usart->base;
        }
    }
    *minor = -1; //TODO: add error code for this
    return NULL;
}

//destroy a tty instance
int usart_destroy(uint8_t minor)
{
    struct usart_device* usart = &usart_table[minor & MAX_USART_MSK];
    usart->base.ops = NULL;
    return 0;
}

//lookup a tty instance
struct device* usart_lookup(uint8_t minor)
{
    struct usart_device* usart = &usart_table[minor & MAX_USART_MSK];
    if (usart->base.ops == NULL) {
        return NULL;
    }
    return &usart->base;
}

void usart_global_update()
{
    for (int i = 0; i < MAX_USART_COUNT; i++) {
        struct usart_device* usart = &usart_table[i];
        if (usart->base.ops != NULL) {
            usart->base.ops->update(&usart->base);
        }
    }
}


