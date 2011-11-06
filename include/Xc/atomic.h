#ifndef _ASM_X86_ATOMIC_H
#define _ASM_X86_ATOMIC_H

#include <Xc/types.h>
#include <Xc/spinlock.h>


/* x86/asm/atomic.h */

#define LOCK_PREFIX ""

static inline void atomic_set(atomic_t *v, int i)
{
    v->counter = i;
}

static inline void atomic_add(int i, atomic_t *v)
{
    asm volatile(LOCK_PREFIX "addl %1, %0"
			: "+m"(v->counter)
			: "ir"(i));
}

static inline void atomic_dec(atomic_t *v)
{
    asm volatile(LOCK_PREFIX "decl %0"
			     : "+m"(v->counter));
}

static inline int atomic_read(const atomic_t *v)
{
    return (*(volatile int *)&(v)->counter);
}

static inline int atomic_dec_and_test(atomic_t *v)
{
    unsigned char c;
	asm volatile(LOCK_PREFIX "decl %0; sete %1"
			: "+m" (v->counter), "=qm"(c)
			: : "memory");
	return c != 0;
}

/* end -- x86/asm/atomic.h */

/* asm-generic/atomic-long.h BITS_PER_LONG == 32 */
typedef atomic_t atomic_long_t;

static inline void atomic_long_add(long i, atomic_long_t *l)
{
    atomic_t *v = (atomic_t *)l;
	atomic_add(i, v);
}

static inline void atomic_long_dec(atomic_long_t *l)
{
    atomic_t *v = (atomic_t *)l;
	atomic_dec(v);
}

static inline long atomic_long_read(atomic_long_t *l)
{
    atomic_t *v = (atomic_t *)l;
	return (long)atomic_read(v);
}

/* end -- atomic-long.h */
#endif
