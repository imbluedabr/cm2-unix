#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t dev_t;

#define MAJOR(DEVNO) (DEVNO >> 4)
#define MINOR(DEVNO) (DEVNO & 0xF)

#define MKDEV(MAJOR_NO, MINOR_NO) ((MAJOR_NO << 4) | (MINOR_NO & 0xF))

#define IOCTL_RESET 1

struct termios {
    uint16_t c_cflag;
    uint16_t c_lflag;
};
#define CNLRET (1 << 10)  //treat \n as \r\n
#define CBAUD 0xF       //set the baud rate, baud rates: 110, 134, 1200, 4800, 9600, 38400, 115200
#define CSIZE 0x300     //frame size, options: 5, 6, 7 ,8
#define ICANON (1 << 0)
#define ECHO (1 << 1)

#define IOCTL_TTY_CLEAR 4
#define IOCTL_TTY_SETMODE 5
#define IOCTL_TTY_GETMODE 6

struct sgi_textmode_config {
    bool cursor;
    bool cursor_direction;
    bool blink;
    bool display;
    uint8_t width;
    uint8_t height;
};

struct sgi_pixmode_config {

};

#define IOCTL_SGI_SET_TEXTMODE 8
#define IOCTL_SGI_GET_TEXTMODE 9
#define IOCTL_SGI_SET_PIXMODE 10
#define IOCTL_SGI_GET_PIXMODE 11

#define IOCTL_BASE 16



