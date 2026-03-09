#pragma once

#include <stdint.h>

typedef uint8_t dev_t;

#define MAJOR(DEVNO) (DEVNO >> 4)
#define MINOR(DEVNO) (DEVNO & 0xF)

#define MKDEV(MAJOR_NO, MINOR_NO) ((MAJOR_NO << 4) | (MINOR_NO & 0xF))

#define IOCTL_RESET 1
#define IOCTL_BASE 16


