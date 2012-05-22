#ifndef _XC_RCUPDATE_H
#define _XC_RCUPDATE_H

#include <Xc/preempt.h>

struct rcu_head {
    struct rcu_head *next;
	void (*func)(struct rcu_head *head);
};

#ifdef CONFIG_PREEMPT_RCU

#else

static inline void __rcu_read_lock(void)
{
    preempt_disable();
}

static inline void __rcu_read_unlock(void)
{
    preempt_enable();
}

#endif

#ifdef CONFIG_DEBUG_LOCK_ALLOC

#else

#define rcu_read_acquire() do {} while (0)
#define rcu_read_release() do {} while (0)

#endif

static inline void rcu_read_lock(void)
{
    __rcu_read_lock();
    /* __acquire(RCU); */
	rcu_read_acquire();
}

static inline void rcu_read_unlock(void)
{
    rcu_read_release();
    /* __release(RCU); */
	__rcu_read_unlock();
}

#endif 
