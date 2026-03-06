#pragma once
#include "dev.h"

#define TTY_IOCTL_CLEAR 0
#define TTY_IOCTL_SETCURSOR 1
#define TTY_IOCTL_SETMODE 2
#define TTY_IOCTL_GETMODE 3

#define TTY_MODE_NOECHO (1 << 0)
#define TTY_MODE_RAW (1 << 1)

struct tty_desc {
    dev_t read_dev;
    dev_t write_dev;
};

