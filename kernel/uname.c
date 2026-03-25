#include <kernel/uname.h>
#include <kernel/settings.h>

const struct utsname uname = {
    .sysname = "CM2-UNIX",
    .nodename = "",
    .release = "0.7.0",
    .version = __DATE__,
    .machine = ARCH_TYPE
};


