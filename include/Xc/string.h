#ifndef _XC_STRING_H
#define _XC_STRING_H

#include <stddef.h>
#include <Xc/types.h>

#ifndef __ASSEMBLY__
extern void *memset(void *s, int c, size_t count);
extern void *memcpy(void *dest, const void *src, size_t count);
extern void *memmove(void *dest, const void *src, size_t count);
#endif

#endif
