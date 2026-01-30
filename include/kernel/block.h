#pragma once
#include <kernel/device.h>

struct disk_hw_interface;

struct disk_device {
    struct device base;
    volatile struct disk_hw_interface* disk;
    struct device_request* current_req;
    uint32_t bytes_copied;
};

#define DISK_BATCH_SIZE 16

void gen_disk_init();

struct device* gen_disk_create(int8_t* minor, const void* args);
int gen_disk_destroy(uint8_t minor);
struct device* gen_disk_lookup(uint8_t minor);

int gen_disk_ioctl(struct device* dev, int cmd, void* arg);
void gen_disk_update(struct device* dev);


