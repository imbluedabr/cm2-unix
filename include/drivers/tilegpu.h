#pragma once

#include <stdint.h>
#include <uapi/tilegpu.h>
#include <kernel/device.h>

#define TILEGPU_X           ((volatile uint8_t  *)0xFFFF)
#define TILEGPU_Y           ((volatile uint8_t  *)0xFFFE)
#define TILEGPU_ADDR        ((volatile uint16_t *)0xFFFC)
#define TILEGPU_FX_OPCODE   ((volatile uint8_t  *)0xFFFB)
#define TILEGPU_FX_IMM      ((volatile uint16_t *)0xFFFA)
#define TILEGPU_CONTROLS    ((volatile uint8_t  *)0xFFF9)


struct tilegpu_hw_interface {
    volatile uint8_t* controls;
    volatile uint16_t* fx_imm;
    volatile uint8_t* fx_opcode;
    volatile uint16_t* tile_id;
    volatile uint8_t* y;
    volatile uint8_t* x;
};

struct tilegpu_device {
    struct device base;//inherit from device
    struct tilegpu_hw_interface gpu_interface;
    struct device_request* current_req;
    uint32_t current_pixels_copied;
};

void tilegpu_init();

struct device* tilegpu_create(int8_t* minor, const void* args); //create an instance, we only have one gpu so this should be simple
int tilegpu_destroy(uint8_t minor); //destroy an instance, doesnt do anything except maybe uninitialize the gpu
struct device* tilegpu_lookup(uint8_t minor); //lookup a gpu instance, once again we only have one gpu so we only accept a minor of 0

int tilegpu_ioctl(struct device* dev, int cmd, void* arg); //send a command to the gpu, e.g draw or fxdraw 
void tilegpu_update(struct device* dev); //this gets called every tick, look at the current request, check if its a read or a write request, do a single operation e.g write a single character and do the bytes_copied++, this way you can write to the gpu without freezing the system
void tilegpu_global_update(); //this calls the update of all registered devices, since there is only one you can directly call tilegpu_update
