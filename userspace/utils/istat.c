#include <stdlib.h>
#include <stdio.h>

struct stat statbuff[8];

int main()
{
    struct istat inode_stats = { .buff = statbuff };
    int count = sysctl(2, &inode_stats, 8);
    
    printf("free ino: %x\n", inode_stats.free_inode_count);
    printf("free fd: %x\n", inode_stats.free_descriptor_count);
    
    for (int i = 0; i < count; i++) {
        printf("n: %s\n", statbuff[i].name);
    }

    return 0;
}


