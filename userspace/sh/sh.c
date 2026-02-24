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

const char* prompt = "# ";

void main()
{
    tty0 = open("/dev/tty0", 0);
    STDOUT = tty0;

    while(1) {
        memset(line_buffer, 0, LINE_SIZE);
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
        } else {
            int fileno_vec[] = {
                tty0,
                -1
            };

            //BUG: the return value is not converted to a pid_t(int8_t) properly, this happens somewhere in the kernel im guessing
            int new = exec(line_buffer, (const char**) argv, fileno_vec);
            if (new == -1) {
                printf("sh: %s: command not found\n", argv[0]);
            } else {
                wait(new);
            }
        }
        
    }
    exit(0);
}


