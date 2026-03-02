#include <stddef.h>
#include <lib/stdlib.h>
#include <kernel/device.h>
#include <drivers/tilegpu.h>
#include <uapi/majors.h>

#include <lib/hex.h>

static struct tilegpu_device gpu0;

const struct device_ops tilegpu_ops = {
    .ioctl = &tilegpu_ioctl,
    .update = &tilegpu_update
};

void tilegpu_init()
{
    struct device_driver* tilegpu_driver = &driver_registry[TILEGPU_MAJOR];
    tilegpu_driver->create = &tilegpu_create;
    tilegpu_driver->destroy = &tilegpu_destroy;
    tilegpu_driver->lookup = &tilegpu_lookup;
    tilegpu_driver->update = &tilegpu_global_update;
    tilegpu_driver->name = "generic gpu";
}

struct device* tilegpu_create(int8_t* minor, const void* args)
{
    struct tilegpu_device *gpu = &gpu0;

    if (gpu->base.ops == NULL) {
        struct tilegpu_hw_interface* gpu_interface = (struct tilegpu_hw_interface*) args;

        gpu->base.ops = (struct device_ops *) &tilegpu_ops;
        memcpy(&gpu->gpu_interface, gpu_interface, sizeof(struct tilegpu_hw_interface));
        gpu->base.count = 0;
        gpu->base.head = 0;
        gpu->base.tail = 0;
        *minor = 0;
        return &gpu->base;
    } else {
        *minor = -1; //TODO: add error code for this
        return NULL;
    }
}

int tilegpu_destroy(uint8_t minor)
{
    //we only will deal with one gpu, so ignore minor.
    struct tilegpu_device *gpu = &gpu0;
    gpu->base.ops = NULL;
    memset(&gpu->gpu_interface, 0, sizeof(gpu->gpu_interface));
    return 0;
}

struct device* tilegpu_lookup(uint8_t minor)
{
    struct tilegpu_device *gpu = &gpu0;
    if (gpu->base.ops == NULL) {
        return NULL;
    }
    return &gpu->base;
}

int tilegpu_ioctl(struct device* dev, int cmd, void* arg)
{
    struct tilegpu_device* gpu = (struct tilegpu_device*) dev;
    struct tilegpu_hw_interface* tilegpu_interface = &gpu->gpu_interface;

    int return_code = 0;
    if (gpu->base.ops == NULL) {
        return 0;
    }

    if (cmd == TILEGPU_IOCTL_CLEAR) {
        *tilegpu_interface->controls = TILEGPU_CLEAR;
    } else if (cmd == TILEGPU_IOCTL_DRAWTILE) {
        struct tilegpu_ioctl_msg_drawtile* msg = (struct tilegpu_ioctl_msg_drawtile *) arg;
        *tilegpu_interface->x = msg->x;
        *tilegpu_interface->y = msg->y;
        *tilegpu_interface->controls = msg->controls;
        *tilegpu_interface->tile_id = msg->tile_id;
    } else {
        return_code = -1;
    }
    return return_code;
}

static inline uint8_t tilegpu_puts(
        struct tilegpu_device* gpu,
        struct device_request* current_req,
        volatile struct tilegpu_hw_interface* gpu_interface
        )
{
    uint32_t i = gpu->current_pixels_copied;
    char c = ((char*) current_req->buffer)[i];

    if (c == '\n' || *gpu_interface->x >= 48) {
        *gpu_interface->x = 0;
        *gpu_interface->y += 2;
    } else {  
        *gpu_interface->tile_id = c;
        //(*gpu_interface->x)++;
        // TODO: fix me
        *gpu_interface->x = i;
        *gpu_interface->controls = TILEGPU_DRAW;
    }

    gpu->current_pixels_copied = ++i;

    if (i == current_req->count) {
        return 1;
    }
    return 0;
}

void tilegpu_update(struct device* dev)
{
    struct tilegpu_device* gpu = (struct tilegpu_device*) dev;
    struct tilegpu_hw_interface* gpu_interface = &gpu->gpu_interface;
    struct device_request* current_req = gpu->current_req;

    if (current_req == NULL) {
        current_req = device_queue_pop(dev);
        if (current_req == NULL)
            return;

        gpu->current_req = current_req;
        gpu->current_pixels_copied = 0;
    }
    
    uint8_t exit = 0;
    if (current_req->operation == DEVICE_OP_WR) {
        exit = tilegpu_puts(gpu, current_req, gpu_interface);
    }

    if (exit) {
        current_req->state = DEVICE_STATE_FINISHED;
        current_req->count = gpu->current_pixels_copied;
        gpu->current_req = NULL;
    }
}

void tilegpu_global_update()
{
    tilegpu_update(&gpu0.base);
}

// // Draw a tile at the specified (x, y) coordinates with the given tile ID.
// // Coordinates are in pixel units.
// // Apply special effects based on fx_op and fx_imm parameters.

// // fx opcodes
// #define NO_FX       0
// #define AND_COLOR   1
// #define NAND_COLOR  2
// #define XOR_COLOR   3
// #define OR_COLOR    4
// #define NOR_COLOR   5

// void tilegpu_fxdraw(uint8_t x, uint8_t y, uint16_t tile_id, uint8_t fx_op, uint16_t fx_imm)
// {
//     *TILEGPU_FX_OPCODE = fx_op;
//     *TILEGPU_FX_IMM = fx_imm;
//     tilegpu_draw(x, y, tile_id);
// }

// // Clear the entire screen.
// void tilegpu_clearscreen(void)
// {
//     *TILEGPU_CONTROLS = TILEGPU_CLEAR;
// }

// // Print a null-terminated string at the specified (x, y) coordinates.
// void tilegpu_puts(uint8_t x, uint8_t y, const char *str)
// {   
//     uint8_t x2 = x, y2 = y;
//     for (uint16_t i = 0; str[i] != '\0'; i++) 
//     {
//         if (str[i] == '\n') y2+=2;
//         tilegpu_draw(x2++, y2, str[i]);
//         if (x2 >= 48 || str[i] == '\n') 
//         {
//             x2 = 0;
//             y2+=2;
//         }
//     }
// }

// void tilegpu_print(const char *str) {
//     static uint8_t y = 0;
//     tilegpu_puts(0, y+=2, str);
//     if (y == 64) {
//         y = 0;
//         *TILEGPU_CONTROLS = TILEGPU_CLEAR;
//     }
// }
