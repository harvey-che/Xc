#ifndef _XC_TYPES_H
#define _XC_TYPES_H

#ifdef __KERNEL__
#ifndef __ASSEMBLY__

typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long s64;
typedef unsigned long long u64;

typedef int size_t;
#endif
#endif

#ifndef __ASSEMBLY__
typedef signed char __s8;
typedef unsigned char __u8;

typedef signed short __s16;
typedef unsigned short __u16;

typedef signed int __s32;
typedef unsigned int __u32;

typedef signed long long __s64;
typedef unsigned long long __u64;


#endif  /* __ASSEMBLY */

#ifndef __ASSEMBLY__
#ifdef __KERNEL__

typedef u64 phys_addr_t;

typedef phys_addr_t resource_size_t;

typedef struct {
    int counter;
} atomic_t;

struct list_head {
    struct list_head *next, *prev;
};

struct hlist_head {
    struct hlist_node *first;
};

struct hlist_node {
    struct hlist_node *next, **prev;
};

typedef int bool;

typedef int clockid_t;

#define pgoff_t unsigned long

typedef unsigned gfp_t;

typedef int pid_t;

#define DECLARE_BITMAP(name, bits)    \
	unsigned long name[BITS_TO_LONGS(bits)]


#endif  /* __KERNEL__ */
#endif  /* __ASSEMBLY__ */

#endif
