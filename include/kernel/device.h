#pragma once
#include <stdint.h>

typedef uint8_t dev_t;

#define MAJOR(DEVNO) (DEVNO >> 4)
#define MINOR(DEVNO) (DEVNO & 0xF)

#define MKDEV(MAJOR_NO, MINOR_NO) ((MAJOR_NO << 4) | (MINOR_NO & 0xF))

struct device;

struct device_driver {
    struct device* (*create)(int8_t* minor, const void* args); //returns device pointer or a negative errno in minor
    int (*destroy)(uint8_t minor);
    struct device* (*lookup)(uint8_t minor);
    const char* name;
};

struct device_ops {
    uint32_t (*read)(struct device* dev, void* buffer, uint32_t count, uint32_t offset);
    uint32_t (*write)(struct device* dev, const void* buffer, uint32_t count, uint32_t offset);
    int (*ioctl)(struct device* dev, int cmd, void* arg);
};

struct device {
    struct device_ops* ops;
};

#define DEVICE_DRIVER_MAX 16
extern struct device_driver driver_registry[DEVICE_DRIVER_MAX];

struct device* device_create(dev_t* devno, uint8_t major, const void* args);
struct device* device_lookup(dev_t devno);
int device_destroy(dev_t devno);

