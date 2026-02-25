#include <lib/stdlib.h>

uint32_t strlen(const char* str)
{
    int size = 0;
    while (*str++ != '\0') {
        size++;
    }
    return size;
}

uint32_t strnlen(const char* str, uint32_t max_size)
{
    int size = 0;
    while (*str++ != '\0') {
        if (size > max_size) {
            break;
        }
        size++;
    }
    return size;
}


int strncmp( const char * s1, const char * s2, uint32_t n )
{
    while ( n && *s1 && ( *s1 == *s2 ) )
    {
        ++s1;
        ++s2;
        --n;
    }
    if ( n == 0 )
    {
        return 0;
    }
    else
    {
        return ( *(unsigned char *)s1 - *(unsigned char *)s2 );
    }
}

int strncpy(char* dest, char* src, uint32_t size)
{
    int i = 0;
    while((*src != '\0') && (size-- > 0)) {
        *(dest++) = *(src++);
        i++;
    }
    return i;
}

void strlcpy(char* dest, char* src, uint32_t size)
{
    while((*src != '\0') && (size-- > 0)) {
        *(dest++) = *(src++);
    }
    *dest = '\0';
}


[[gnu::used]] void memset(void* dest, uint8_t value, uint32_t size)
{
    while(size-- > 0) {
        ((uint8_t*) dest)[size] = value;
    }
}


[[gnu::used]] void* memcpy(void* dest, void* source, size_t size)
{
    while(size-- > 0) {
        ((uint8_t*) dest)[size] = ((uint8_t*) source)[size];
    }
    return dest;
}


