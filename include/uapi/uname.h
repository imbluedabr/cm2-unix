#pragma once

#define UTSLEN 16
struct utsname {
    char sysname[UTSLEN];    /* Operating system name (e.g., "Linux") */
    char nodename[UTSLEN];   /* Name within communications network
                          to which the node is attached, if any */
    char release[UTSLEN];    /* Operating system release
                          (e.g., "2.6.28") */
    char version[UTSLEN];    /* Operating system version */
    char machine[UTSLEN];    /* Hardware type identifier */
};


