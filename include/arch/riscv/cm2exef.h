#pragma once
#include <stdint.h>

enum cm2exef_isa : uint8_t {
    RV22M0
};

enum cm2exef_abi : uint8_t {
    NIL,
    ILP32,
    ILP32F,
    ILP32D,
    ILP32E
};

struct cm2exef_abi_version {
    uint8_t major;
    uint8_t minor;
};

struct cm2exef_header {
    uint32_t magic; // must be 0xE85250D6
    enum cm2exef_abi abi;    // ILP32 = 1, ILP32F = 2, ILP32D = 3, and ILP32E = 4, zero is invalid.
    enum cm2exef_isa isa;   // what isa this executable is made for.
    struct cm2exef_abi_version version;
    uint32_t program_break; //offset to the end of the program, this includes bss and any other nobits sections
};


