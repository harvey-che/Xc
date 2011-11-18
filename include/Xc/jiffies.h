#ifndef _XC_JIFFIES_H
#define _XC_JIFFIES_H

#include <Xc/types.h>

extern u64 __attribute__((section(".data"))) jiffies_64;
extern unsigned long volatile __attribute__((section(".data"))) jiffies;

/* asm-generic/param.h */
#define HZ 1000
#define LOAD_FREQ (5 * HZ + 1)
/* end */
#define INITIAL_JIFFIES ((unsigned long)(unsigned int)(-300 * HZ))

#endif
