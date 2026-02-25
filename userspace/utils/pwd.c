#include <stdio.h>
#include <stdlib.h>

char cwd[FS_PATH_LEN + 1];
int main() {
    getcwd(cwd, FS_PATH_LEN);

    printf("%s\n", cwd);

    return 0;
}


