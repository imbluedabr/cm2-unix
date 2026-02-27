#include <stdlib.h>
#include <stdio.h>
#include <tty.h>

int tty0;

#define LINE_SIZE 24

char line_buffer[LINE_SIZE];

char* argv[5];

int parse_args(char* buffer, int size)
{
    memset(argv, 0, sizeof(argv));
    if (size == 0) { return 0; }
    int argc = 0;
    char* current_arg = buffer;
    for (int i = 0; i < size; i++) {
        if (argc > 4) {
            return argc;
        }
        if (buffer[i] == ' ') {
            buffer[i] = '\0';
            argv[argc++] = current_arg;
            current_arg = &buffer[i + 1];
        }
    }
    argv[argc++] = current_arg;

    return argc;
}

const char* prompt = "$ ";
char cwd[FS_PATH_LEN + 1];

void main()
{
    tty0 = open("/dev/tty0", 0);
    stdout = tty0;
    
    while(1) {
        memset(line_buffer, 0, LINE_SIZE);
        getcwd(cwd, FS_PATH_LEN);
        puts(cwd);
        puts(prompt);

        int count = read(tty0, line_buffer, LINE_SIZE);
        int argc = parse_args(line_buffer, count);

        if (argc == 0) {
            continue;
        }

        if (strncmp(argv[0], "exit", LINE_SIZE) == 0) {
            break;
        } else if (strncmp(argv[0], "clear", LINE_SIZE) == 0) {
            ioctl(tty0, TTY_IOCTL_CLEAR, NULL);
        } else if (strncmp(argv[0], "cd", LINE_SIZE) == 0) {
            if (argv[1] == NULL) {
                puts("cd: missing argument\n");
            } else if (chdir(argv[1]) < 0) {
                puts("cd: path not found\n");
            }
        } else {
            int fileno_vec[] = {
                tty0,
                -1
            };

            //BUG: the return value is not converted to a pid_t(int8_t) properly, this happens somewhere in the kernel im guessing
            int new;
            int off = strncpy(cwd, "/bin/", 5);
            strlcpy(cwd + off, argv[0], FS_INAME_LEN);
            
            new = exec(cwd, (const char**) argv, fileno_vec);
            if (new == -1) { //if it wasnt found in /bin we look in the current working directory of the shell
                new = exec(argv[0], (const char**) argv, fileno_vec);
            }
            if (new == -1) {
                printf("sh: %s: command not found\n", argv[0]);
            } else {
                wait(new); //wait for the process to finish
            }
        }
        
    }
    exit(0);
}


