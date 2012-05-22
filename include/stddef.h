#ifndef _XC_STDDEF_H
#define _XC_STDDEF_H

#ifndef __ASSEMBLY__

#undef NULL
#if defined(__cplusplus)
#define NULL 0
#else
#define NULL ((void*)0)
#endif

//#ifdef __KERNEL__

enum {
    false = 0,
	true = 1
};

#undef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

//#endif

#endif

#endif
