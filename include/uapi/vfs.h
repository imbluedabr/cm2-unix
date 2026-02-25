#pragma once
#include <stdint.h>
#include "dev.h"

typedef uint32_t ino_t; //this is a unique identifier for an inode, this would be like the cluster + offset in fat
#define FS_MODE_DIR 0
#define FS_MODE_FILE 2
#define FS_MODE_MOUNT 1
#define FS_MODE_DEV 3
#define FS_IS_A_FILE(MODE) (MODE & 0b10)
#define FS_INAME_LEN 9
#define FS_PATH_LEN (FS_INAME_LEN*4 - 1)

struct stat {
    uint32_t size;
    uint32_t time;
    uint8_t mode;
    dev_t dev;
    ino_t d_ino;
    char name[FS_INAME_LEN + 1];
};

//file flag defenitions
#define FD_R (1 << 0)
#define FD_W (1 << 1)
#define FD_CREAT (1 << 2)
#define FD_NONBLOCK (1 << 3)


