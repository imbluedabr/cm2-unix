#pragma once
#include <stdint.h>

typedef int8_t pid_t;

enum proc_state : uint8_t {
    BLOCKED,
    READY,
    DEAD,
    UNALLOCATED
};

struct procinfo {
    pid_t upid;
    enum proc_state state;
    uint32_t program_base;
    uint32_t program_size;
};

