#include <kernel/device.h>
#include <stddef.h>
#include <lib/kprint.h>

struct device_driver driver_registry[DEVICE_DRIVER_MAX];

struct device_request request_table[DEVICE_REQ_TBL_LEN];
uint8_t request_table_free[DEVICE_REQ_TBL_LEN];
uint8_t request_table_free_len;

int device_queue_action(struct device* dev, struct device_request* req)
{
    if (dev->count == DEVICE_MAX_REQ) return -1;
    
    dev->request_queue[dev->tail] = req;
    dev->tail = (dev->tail + 1) & DEVICE_MAX_REQ_MSK;
    dev->count++;

    return 0;
}

struct device_request* device_queue_pop(struct device* dev)
{
    if (dev->count == 0) return NULL;
    
    struct device_request* req = dev->request_queue[dev->head];
    dev->head = (dev->head + 1) & DEVICE_MAX_REQ_MSK;
    dev->count--;
    
    return req;
}

struct device_request* device_newreq(void* buffer, uint32_t count, uint32_t offset, uint8_t operation) {
    uint8_t tmp = request_table_free_len;
    if (tmp == 0) return NULL;
    uint8_t index = request_table_free[--tmp];
    struct device_request* new = &request_table[index];
    request_table_free_len = tmp;

    new->buffer = buffer;
    new->count = count;
    new->index = index;
    new->operation = operation;
    new->offset = offset;
    new->state = DEVICE_STATE_PENDING;
    
    return new;
}

void device_free_req(struct device_request* req) {
    req->state = DEVICE_STATE_PENDING;
    request_table_free[request_table_free_len++] = req->index;
}


struct device* device_create(dev_t* devno, uint8_t major, const void* args)
{
    struct device_driver* driver = &driver_registry[major];
    if (driver->create == NULL) {
        return NULL;
    }
    
    int8_t minor;
    struct device* device_instance = driver->create(&minor, args);
    
    if (minor < 0) {
        return NULL; //TODO: return the error code somehow
    }

    *devno = MKDEV(major, minor);
    return device_instance;
}

struct device* device_lookup(dev_t devno)
{
    uint8_t major = MAJOR(devno);
    uint8_t minor = MINOR(devno);
    if (major >= DEVICE_DRIVER_MAX) {
        return NULL;
    }
    struct device_driver* driver = &driver_registry[major];
    if (driver->lookup == NULL) {
        return NULL;
    }

    //TODO: we must return an error code somehow
    struct device* device_instance = driver->lookup(minor);
    return device_instance;
}

int device_destroy(dev_t devno)
{
    uint8_t major = MAJOR(devno);
    uint8_t minor = MINOR(devno);
    struct device_driver* driver = &driver_registry[major];
    if (driver->destroy == NULL) {
        return -1; //TODO: create error code for this
    }

    return driver->destroy(minor);
}

void device_init() {
    request_table_free_len = DEVICE_REQ_TBL_LEN;
    for (int i = 0; i < DEVICE_REQ_TBL_LEN; i++) {
        request_table_free[i] = i;
    }
}

void device_update() {
    for (int i = 0; i < DEVICE_DRIVER_MAX; i++) {
        struct device_driver* driver = &driver_registry[i];
        if (driver->update == NULL) {
            continue;
        }
        driver->update();
    }
}

void debug(char chr) {
    *((volatile char*) 0xFFF5) = chr;
    *((volatile uint8_t*) 0xFFF4) = 1;
    *((volatile uint8_t*) 0xFFF6) = *((uint8_t*)0xFFF6) + 1;
}

