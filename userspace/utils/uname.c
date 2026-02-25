#include <stdio.h>
#include <stdlib.h>

int main(const char** argv)
{
    char uname[48];
    sysctl(1, uname, 48);
    
    puts(uname);

    return 0;
}

