#pragma once
#include <stdint.h>


enum Syscalls {
    SYS_OPEN,
    SYS_READ,
    SYS_WRITE,
    SYS_CLOSE,
    SYS_IOCTL,
    SYS_FSTAT,
    SYS_READDIR,
    SYS_GETCWD,
    SYS_CHDIR,
    SYS_YIELD,
    SYS_EXEC,
    SYS_EXIT,
    SYS_KILL,
    SYS_WAIT,
    SYS_MOUNT,
    SYS_UMOUNT,
    SYS_SYSCTL
};


typedef int (*syscall_t)(uint32_t a, uint32_t b, uint32_t c, uint32_t d);
static const syscall_t SYSCALL = (syscall_t)0x00000090;


