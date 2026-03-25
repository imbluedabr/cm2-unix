#include <stdio.h>
#include <stdlib.h>
#include <uname.h>

int main(const char** argv)
{
    struct utsname buff;
    sysctl(1, &buff, 0);
    
    printf("%s %s %s %s\n", buff.sysname, buff.release, buff.version, buff.machine);

    return 0;
}

