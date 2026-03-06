#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <tty.h>

int tty0;

#define LINE_SIZE 24

char line_buffer[LINE_SIZE];

char* new_argv[5];

int parse_args(char* buffer, int size)
{
    memset(new_argv, 0, sizeof(new_argv));
    if (size == 0) { return 0; }
    int argc = 0;
    char* current_arg = buffer;
    for (int i = 0; i < size; i++) {
        if (argc > 4) {
            return argc;
        }
        if (buffer[i] == ' ') {
            buffer[i] = '\0';
            new_argv[argc++] = current_arg;
            current_arg = &buffer[i + 1];
        }
    }
    new_argv[argc++] = current_arg;

    return argc;
}

const char* prompt = "$ ";
char cwd[FS_PATH_LEN + 1];

int main(const char** argv)
{
    
    if (argv != NULL) {
        if (argv[1] != NULL) {
            stdout = open(argv[1], 0);
        }
    } else {
        stdout = open("/dev/tty0", 0);
    }
    
    while(1) {
        memset(line_buffer, 0, LINE_SIZE);
        getcwd(cwd, FS_PATH_LEN);
        puts(cwd);
        puts(prompt);

        int count = read(stdout, line_buffer, LINE_SIZE);
        int new_argc = parse_args(line_buffer, count);

        if (new_argc == 0) {
            continue;
        }
        bool background_execution = false;
        if (strncmp(new_argv[new_argc - 1], "&", 2) == 0) {
            new_argv[--new_argc] = NULL;
            background_execution = true;
        }

        if (strncmp(new_argv[0], "exit", LINE_SIZE) == 0) {
            break;
        } else if (strncmp(new_argv[0], "clear", LINE_SIZE) == 0) {
            puts("\e[J");
        } else if (strncmp(new_argv[0], "cd", LINE_SIZE) == 0) {
            if (new_argv[1] == NULL) {
                puts("cd: missing argument\n");
            } else if (chdir(new_argv[1]) < 0) {
                puts("cd: path not found\n");
            }
        } else {
            int fileno_vec[] = {
                stdout,
                -1
            };

            //BUG: the return value is not converted to a pid_t(int8_t) properly, this happens somewhere in the kernel im guessing
            int new;
            int off = strncpy(cwd, "/bin/", 5);
            strlcpy(cwd + off, new_argv[0], FS_INAME_LEN);
            
            new = exec(cwd, (const char**) new_argv, fileno_vec);
            if (new == -1) { //if it wasnt found in /bin we look in the current working directory of the shell
                new = exec(new_argv[0], (const char**) new_argv, fileno_vec);
            }
            if (new == -1) {
                printf("sh: %s: command not found\n", new_argv[0]);
            } else if (!background_execution) {
                wait(new); //wait for the process to finish
            } else {
                printf("PID: %x\n", new);
            }
        }
        
    }
    return 0;
}


