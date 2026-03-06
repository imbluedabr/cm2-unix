#include <kernel/devtbl.h>
#include <kernel/device.h>
#include <lib/kprint.h>
#include <lib/stdlib.h>
#include <fs/devfs.h>

const char *device_handle_name[] = {
    "ttyx",
    "usartx",
    "diskx",
    "gpux"
};

void devtbl_init(device_node_t* device_config, int size)
{
    for (int i = 0; i < size; i++) {
        device_node_t* node = &device_config[i];
        dev_t devno;
        if (!device_create(&devno, node->major, node->arg)) {
            kprintf("error: failed to create dev %x, at pos %x in devtbl\n", node->major, i);
        } else {
            char buff[7] = {0};
            int numi = strncpy(buff, (char*) device_handle_name[node->major], 6);
            buff[numi - 1] = '0' + MINOR(devno);
            devfs_create_handle(buff, devno);
        }
    }
}



