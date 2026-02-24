#include <stdio.h>
#include <stdlib.h>
#include <proc.h>

void main()
{
    STDOUT = 0;
    struct procinfo buff[4];

    int count = sysctl(0, buff, 4);
    if (count == -1) {
        exit(-1);
    }
    puts("pid state base size\n");
    for (int i = 0; i < count; i++) {
        struct procinfo* process = &buff[i];
        char info[] = "xx  xx    xxxx xxxx\n";
        int_to_hex(info, process->upid, 2);
        int_to_hex(info + 4, process->state, 2);
        int_to_hex(info + 10, process->program_base, 4);
        int_to_hex(info + 15, process->program_size, 4);
        puts(info);
    }
    exit(0);
}

