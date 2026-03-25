#pragma once
#include <stdint.h>
#include <uapi/dev.h>


#define DEVICE_OP_RD 0
#define DEVICE_OP_WR 1
#define DEVICE_STATE_FINISHED 255
#define DEVICE_STATE_PENDING 0

struct device_request {
    void* buffer;
    uint16_t count;
    uint8_t index;
    uint8_t operation;
    uint32_t offset : 24;
    uint8_t state : 8;
};

struct device;

struct device_driver {
    struct device* (*create)(int8_t* minor, const void* args); //returns device pointer or a negative errno in minor
    int (*destroy)(uint8_t minor);
    struct device* (*lookup)(uint8_t minor);
    void (*update)(); //update all instances of a driver
    const char* name;
};

struct device_ops {
    int (*readb)(struct device* dev);
    int (*writeb)(struct device* dev, uint8_t val);
    int (*ioctl)(struct device* dev, int cmd, void* arg);
    void (*update)(struct device* dev);
};

#define DEVICE_MAX_REQ 4
#define DEVICE_MAX_REQ_MSK (DEVICE_MAX_REQ-1)
struct device {
    struct device_ops* ops;
    struct device_request *request_queue[DEVICE_MAX_REQ];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
};

//this sends a request to a driver
int device_queue_action(struct device* dev, struct device_request* req);
//this is only for internal driver use
struct device_request* device_queue_pop(struct device* dev);

#define DEVICE_DRIVER_MAX 6
extern struct device_driver driver_registry[DEVICE_DRIVER_MAX];
#define DEVICE_REQ_TBL_LEN 6

struct device_request* device_newreq(void* buffer, uint32_t count, uint32_t offset, uint8_t operation);
void device_free_req(struct device_request* req); 
struct device* device_create(dev_t* devno, uint8_t major, const void* args);
struct device* device_lookup(dev_t devno);
int device_destroy(dev_t devno);
void device_init();
void device_update(); //global device update



