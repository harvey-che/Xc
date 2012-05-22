#ifndef _XC_BIT_SPINLOCK_H
#define _XC_BIT_SPINLOCK_H

#include <Xc/preempt.h>

static inline void bit_spin_lock(int bitnum, unsigned long *addr)
{
    preempt_disable();
}


static inline void __bit_spin_unlock(int bitnum, unsigned long *addr)
{
    preempt_enable();
}
#endif
