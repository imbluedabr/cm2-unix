
# Syscalls

| syscallno | return type | name    | arg1                 | arg2                 | arg3             | implemented |
|-----------|-------------|---------|----------------------|----------------------|------------------|-------------|
| 0         | int         | open    | const char* path     | uint8_t flags        |                  | yes         |
| 1         | int         | read    | int fd               | void* buffer         | int count        | yes         |
| 2         | int         | write   | int fd               | void* buffer         | int count        | yes         |
| 3         | void        | close   | int fd               |                      |                  | yes         |
| 4         | int         | ioctl   | int fd               | int cmd              | void* arg        | yes         |
| 5         | int         | fstat   | int fd               | struct stat* buff    |                  | yes         |
| 6         | int         | readdir | int fd               | struct stat* buff    | int count        | yes         |
| 7         | int         | getcwd  | char* buff           | int size             |                  | yes         |
| 8         | int         | chdir   | const char* path     |                      |                  | tes         |
| 9         | void        | yield   |                      |                      |                  | yes         |
| 10        | pid_t       | exec    | const char* path     | const char** argv    | int* fileno_vec  | yes         |
| 11        | void        | exit    | int error_code       |                      |                  | yes         |
| 12        | int         | kill    | pid_t upid           |                      |                  | yes         |
| 13        | int         | wait    | pid_t upid           |                      |                  | yes         |
| 14        | int         | mount   | const char* dev_path | const char* dir_path | const char* type | no          |
| 15        | int         | umount  | const char* dir_path |                      |                  | no          |
| 16        | int         | sysctl  | int cmd              | void* buff           | int count        | yes         |




