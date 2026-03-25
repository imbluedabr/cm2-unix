#pragma once

#include <kernel/device.h>

#define ST7920_CMD_BUFF_SIZE 16

struct st9720_cmd {
    uint8_t data;
    uint8_t rs;
};

struct st7920_device {
    struct device base;
    struct device_request* current_req;
    struct st9720_cmd cmd_buff[ST7920_CMD_BUFF_SIZE];
    uint8_t cmd_tail;
    uint8_t cmd_head;
    uint16_t bytes_copied;
    uint8_t cursor_pos;
    uint8_t mode;
};

//register the tty driver
void st7920_init();

//create a tty instance, args is a tty_hardware_interface pointer to the tty
struct device* st7920_create(int8_t* minor, const void* args);

//destroy a tty instance
int st7920_destroy(uint8_t minor);

//lookup a tty insance
struct device* st7920_lookup(uint8_t minor);

void st7920_global_update();

int st7920_writeb(struct device* dev, uint8_t val);
int st7920_readb(struct device* dev);

int st7920_ioctl(struct device* dev, int cmd, void* arg);

void st7920_update(struct device* dev);








