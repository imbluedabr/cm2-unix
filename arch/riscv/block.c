#include <stddef.h>
#include <kernel/block.h>
#include <kernel/majors.h>

struct disk_hw_interface {
    uint8_t write;
    uint8_t out;
    uint8_t _padding1;
    uint8_t in;
    uint8_t _padding2;
    uint16_t adres;
};

const struct device_ops disk_ops = {
    .ioctl = &gen_disk_ioctl,
    .update = &gen_disk_update
};

//we only suport one instance to save memory
struct disk_device disk0;

void gen_disk_init()
{
    struct device_driver* gen_disk_driver = &driver_registry[GEN_DISK_MAJOR];
    gen_disk_driver->create = &gen_disk_create;
    gen_disk_driver->destroy = &gen_disk_destroy;
    gen_disk_driver->lookup = &gen_disk_lookup;
    gen_disk_driver->name = "generic block device";
}

struct device* gen_disk_create(int8_t* minor, const void* args)
{
    if (disk0.base.ops == NULL) {
        *minor = -1;
        return NULL;
    }

    disk0.base.ops = (struct device_ops*) &disk_ops;
    disk0.base.count = 0;
    disk0.base.head = 0;
    disk0.base.tail = 0;
    disk0.bytes_copied = 0;
    disk0.current_req = NULL;
    disk0.disk = (struct disk_hw_interface*) args;
    *minor = 0;
    return &disk0.base;
}

int gen_disk_destroy(uint8_t minor)
{
    if (minor != 0) {
        return -1;
    }

    disk0.base.ops = NULL;
    return 0;
}

struct device* gen_disk_lookup(uint8_t minor)
{
    if (minor != 0 || disk0.base.ops == NULL) {
        return NULL;
    }
    return &disk0.base;
}



int gen_disk_ioctl(struct device* dev, int cmd, void* arg)
{
    //no special commands for this hardware
    return 0;
}

static inline uint8_t gen_disk_write(
        struct disk_device* disk,
        struct device_request* current_req
        )
{
    volatile struct disk_hw_interface* hw = disk->disk;
    uint32_t i = disk->bytes_copied;
    
    for (int k = 0; k < DISK_BATCH_SIZE; k++) {
        hw->adres = i + current_req->offset;
        hw->in = ((uint8_t*) current_req->buffer)[i];
        hw->write = 1;

        if (++i == current_req->count) {
            disk->bytes_copied = i;
            return 1;
        }
    }

    disk->bytes_copied = i;
    return 0;

}

static inline uint8_t gen_disk_read(
        struct disk_device* disk,
        struct device_request* current_req
        )
{
    volatile struct disk_hw_interface* hw = disk->disk;
    uint32_t i = disk->bytes_copied;
    
    for (int k = 0; k < DISK_BATCH_SIZE; k++) {
        hw->adres = i + current_req->offset;
        ((uint8_t*) current_req->buffer)[i] = hw->out;

        if (++i == current_req->count) {
            disk->bytes_copied = i;
            return 1;
        }
    }

    disk->bytes_copied = i;
    return 0;
}


void gen_disk_update(struct device* dev)
{
    struct disk_device* disk = (struct disk_device*) dev;
    struct device_request* current_req = disk->current_req;

    if (current_req == NULL) {
        current_req = device_queue_pop(dev);
        if (current_req == NULL)
            return;

        disk->current_req = current_req;
        disk->bytes_copied = 0;
    }
    
    uint8_t exit;
    if (current_req->operation == DEVICE_OP_WR) {
        exit = gen_disk_write(disk, current_req);
    } else {
        exit = gen_disk_read(disk, current_req);
    }

    if (exit) {
        current_req->state = DEVICE_STATE_FINISHED;
        current_req->count = disk->bytes_copied;
        disk->current_req = NULL;
    }

   
} 

