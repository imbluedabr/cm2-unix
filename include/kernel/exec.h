#pragma once
#include <kernel/proc.h>


int proc_exec(exe_t* exec_state, const char* path, const char** argv, int* fileno_vec);
int proc_exec_update(exe_t* exec_state);

