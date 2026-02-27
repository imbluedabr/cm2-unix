#include <stdint.h>

extern uint8_t stdout;
extern uint8_t stdin;

void putc(char c);
void puts(const char *str);
void printf(const char *fmt, ...);
int fgets(char* buffer, int size, int fd);


