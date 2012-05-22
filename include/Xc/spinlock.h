#ifndef _XC_SPINLOCK_H
#define _XC_SPINLOCK_H

#include <Xc/irqflags.h>
#include <Xc/spinlock_types.h>
#include <Xc/compiler-gcc.h>
#include <Xc/preempt.h>
#include <Xc/typecheck.h>


/* spinlock_api_up.h */
/* In UP system, spin_lock just disables kernel preemption */

#define assert_raw_spin_locked(lock) do { (void)(lock); } while (0)

#define __LOCK(lock)    \
	do { preempt_disable(); (void)(lock); } while (0)

#define __UNLOCK(lock)    \
	do { preempt_enable(); (void)(lock); } while (0)

#define __LOCK_IRQSAVE(lock, flags)    \
	do { local_irq_save(flags); __LOCK(lock); } while (0)

#define __UNLOCK_IRQRESTORE(lock, flags)    \
	do { local_irq_restore(flags); __UNLOCK(lock); } while (0)

#define _raw_spin_lock(lock) __LOCK(lock)
#define _raw_spin_unlock(lock) __UNLOCK(lock)

#define _raw_spin_lock_irqsave(lock, flags)  __LOCK_IRQSAVE(lock, flags)
#define _raw_spin_unlock_irqrestore(lock, flags)  __UNLOCK_IRQRESTORE(lock, flags)
/* end -- spinlock_api_up.h */


static inline raw_spinlock_t *spinlock_check(spinlock_t *lock)
{
    return &lock->rlock;
}

#define raw_spin_lock_init(lock)    \
	do { *(lock) = __RAW_SPIN_LOCK_UNLOCKED(lock); } while (0)


#define raw_spin_lock(lock) _raw_spin_lock(lock)
#define raw_spin_unlock(lock) _raw_spin_unlock(lock)

#define raw_spin_lock_irqsave(lock, flags)    \
	do {                        \
		typecheck(unsigned long, flags);      \
		_raw_spin_lock_irqsave(lock, flags);  \
	} while (0)

#define raw_spin_unlock_irqrestore(lock, flags)    \
	do {                         \
		typecheck(unsigned long, flags);      \
		_raw_spin_unlock_irqrestore(lock, flags); } while (0)

#define spin_lock_init(_lock)     \
	do {                          \
		spinlock_check(_lock);    \
		raw_spin_lock_init(&(_lock)->rlock);    \
	} while (0)

static inline void spin_lock(spinlock_t *lock)
{
    raw_spin_lock(&lock->rlock);
}

static inline void spin_unlock(spinlock_t *lock)
{
    raw_spin_unlock(&lock->rlock);
}

#define spin_lock_irqsave(lock, flags)    \
	do {                                  \
		raw_spin_lock_irqsave(spinlock_check(lock), flags);    \
	} while (0)

static inline void spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags)
{
    raw_spin_unlock_irqrestore(&lock->rlock, flags);
}

#include <Xc/atomic.h>

#endif
