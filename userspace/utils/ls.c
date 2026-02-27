#include <stdlib.h>
#include <stdio.h>
#include <vfs.h>
#include <stddef.h>

struct stat dirbuff[8];
const char* ftypes = "dmfn";

char cwd[FS_PATH_LEN + 1];

int main(const char** argv)
{   
    getcwd(cwd, FS_PATH_LEN);
    const char* path = cwd;
    if (argv[1] != NULL) {
        path = argv[1];
    }

    int dirfd = open(path, 0);
    int count = readdir(dirfd, dirbuff, 8);

    for (int i = 0; i < count; i++) {
        char buff[] = "x           xxxx\n";
        struct stat* dir = &dirbuff[i];
        buff[0] = ftypes[dir->mode];
        int_to_hex(buff + 12, dir->size, 4);
        strncpy(buff + 2, dir->name, FS_INAME_LEN);
        write(stdout, buff, sizeof(buff) - 1);
    }

    
    return 0;
}


