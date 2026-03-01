#pragma once

#define TTY_IOCTL_CLEAR 0
#define TTY_IOCTL_SETCURSOR 1
#define TTY_IOCTL_SETMODE 2
#define TTY_IOCTL_GETMODE 3

#define TTY_MODE_NOECHO (1 << 0)
#define TTY_MODE_RAW (1 << 1)

