#include <kernel/device.h>

struct tty_hardware_interface;

struct tty_device {
    struct device base;
    volatile struct tty_hardware_interface* tty;
    struct device_request* current_req;
    uint32_t current_bytes_copied;
};

#define TTY_IOCTL_CLEAR 0
#define TTY_IOCTL_SETCURSOR 1

//register the tty driver
void tty_init();

//create a tty instance, args is a tty_hardware_interface pointer to the tty
struct device* tty_create(int8_t* minor, const void* args);

//destroy a tty instance
int tty_destroy(uint8_t minor);

//lookup a tty insance
struct device* tty_lookup(uint8_t minor);

void tty_global_update();

int tty_ioctl(struct device* dev, int cmd, void* arg);

void tty_update(struct device* dev);

