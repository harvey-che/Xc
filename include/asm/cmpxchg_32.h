#ifndef _ASM_X86_CMPXCHG_32_H
#define _ASM_X86_CMPXCHG_32_H

#define cmpxchg8b(ptr, o1, o2, n1, n2)    \
	({                                    \
	     char __ret;                      \
	     __typeof__(o2) __dummy;          \
	     __typeof__(*(ptr)) __old1 = (o1);    \
	     __typeof__(o2) __old2 = (o2);    \
	     __typeof__(*(ptr)) __new1 = (n1);    \
	     __typeof__(o2) __new2 = (n2);    \
	     asm volatile(LOCK_PREFIX "cmpxchg8b %2; setz %1"    \
                      : "=d"(__dummy), "=a"(__ret), "+m"(*ptr)    \
			          : "a"(__old1), "d"(__old2), "b"(__new1), "c"(__new2)    \
			          : "memory");        \
	     __ret; })

#define cmpxchg_double(ptr, o1, o2, n1, n2)   \
	({                                        \
	     cmpxchg8b((ptr), (o1), (o2), (n1), (n2));    \
	 })

#define system_has_cmpxchg_double() (1)
#endif
