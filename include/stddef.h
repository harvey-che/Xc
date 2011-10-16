#ifndef _XC_STDDEF_H
#define _XC_STDDEF_H

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

//#endif

#endif
