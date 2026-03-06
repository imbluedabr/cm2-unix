#pragma once

typedef struct {
    int major;
    const void* arg;
} device_node_t;

void devtbl_init(device_node_t* device_config, int size);

