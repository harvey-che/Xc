#ifndef _ASM_GENERIC_TYPES_H
#define _ASM_GENERIC_TYPES_H

#ifndef __ASSEMBLY__
typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long s64;
typedef unsigned long u64;


typedef signed char __s8;
typedef unsigned char __u8;

typedef signed short __s16;
typedef unsigned short __u16;

typedef signed int __s32;
typedef unsigned int __u32;

typedef signed long __s64;
typedef unsigned long __u64;
#endif  /* __ASSEMBLY */

typedef int size_t;

#endif
