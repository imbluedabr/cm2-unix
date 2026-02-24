#pragma once
#include <kernel/proc.h>


//pid_t exec(const char* path, const char** argv, int* fileno_vec);
void sys_exec();
void sys_exec_update(struct proc* process);

