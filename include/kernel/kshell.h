#pragma  once
#include <stdint.h>

#define KSHELL_STACK_SIZE 64
[[gnu::aligned(16)]] extern uint8_t kshell_thread_stack[KSHELL_STACK_SIZE];

void kshell_thread();

