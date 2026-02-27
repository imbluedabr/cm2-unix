#include <stdlib.h>
#include <stdio.h>

char buff[256];

void main(const char** argv)
{
    const char* path = argv[1];
    if (path == NULL) {
        exit(-1);
    }

    int file = open(path, FD_R);
    if (file == -1) {
        puts("cat: file not found\n");
        exit(-1);
    }
    int count = read(file, buff, 256);
    write(stdout, buff, strnlen(buff, count));

    exit(0);
}

