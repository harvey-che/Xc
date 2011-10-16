#ifndef _XC_STRING_H
#define _XC_STRING_H

#include <stddef.h>

#if __GNUC__ >= 4
#define memset(s,c,count) __builtin_memset(s,c,count)
#else
#error "No __builtin_memset available"
#endif

void * memcpy(void *to, void *from, size_t n)
{
    return to;
}
#endif
