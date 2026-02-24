#include <kernel/kshell.h>
#include <kernel/syscall.h>
#include <stddef.h>

[[gnu::aligned(16)]] uint8_t kshell_thread_stack[KSHELL_STACK_SIZE];

void exit(int exit_code)
{
    syscall(SYS_EXIT, exit_code, 0, 0);
}

pid_t exec(const char* path, const char** argv, int* fd_vec)
{
    return syscall(SYS_EXEC, (uint32_t) path, (uint32_t) argv, (uint32_t) fd_vec);
}

void kshell_thread()
{   
    exec("/bin/sh", NULL, NULL);
    
    exit(0);
}

