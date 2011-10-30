#ifndef _XC_BITOPS_H
#define _XC_BITOPS_H

/*
 * linux/bitops.h -> asm/bitops.h
 */

#include <Xc/compiler-gcc.h>
#include <asm/alternative.h>

#define BITOP_ADDR(x) "+m" (*(volatile long *)(x))
#define ADDR BITOP_ADDR(addr)

#define ADDR BITOP_ADDR(addr)

#define IS_IMMEDIATE(nr)  (__builtin_constant_p(nr))
#define CONST_MASK_ADDR(nr, addr)  BITOP_ADDR((void *)(addr) + ((nr) >> 3))
#define CONST_MASK(nr)  (1 << ((nr) & 7))

/*
 * __xxx-xxx non-atomic and can be ordered
 * xxx_xxx atomic and MAY NOT be ordered, but you can't rely on it on non-x86 architecture
 */

/* What's the effect of the key word "volatile" here */
static void set_bit(unsigned int nr, volatile unsigned long *addr)
{
    if (IS_IMMEDIATE(nr)) {
        asm volatile(LOCK_PREFIX "orb %1, %0"
				: CONST_MASK_ADDR(nr, addr)
				: "iq"((u8)CONST_MASK(nr))
				: "memory");
	} else {
        asm volatile(LOCK_PREFIX "bts %1, %0"
				: BITOP_ADDR(addr) : "Ir"(nr) : "memory");
	}
}

static inline void __set_bit(int nr, volatile unsigned long *addr)
{
    asm volatile("bts %1, %0" : ADDR : "Ir"(nr) : "memory");
}

static inline void clear_bit(int nr, volatile unsigned long *addr)
{
    if (IS_IMMEDIATE(nr)) {
        asm volatile(LOCK_PREFIX "andb %1, %0"
				: CONST_MASK_ADDR(nr, addr)
				: "iq"((u8)~CONST_MASK(nr)));
	} else {
        asm volatile(LOCK_PREFIX "btr %1, %0"
				: BITOP_ADDR(addr)
				: "Ir" (nr));
	}
}

static inline void clear_bit_unlock(unsigned nr, volatile unsigned long *addr)
{
    barrier();
	clear_bit(nr, addr);
}

static inline void __clear_bit(int nr, volatile unsigned long *addr)
{
    asm volatile("btr %1, %0" : ADDR : "Ir"(nr));
}

static inline void __clear_bit_unlock(unsigned nr, volatile unsigned long *addr)
{
    barrier();
	__clear_bit(nr, addr);
}

#define BITS_PER_LONG 32

#define BIT_MASK(nr) (1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr) ((nr) / BITS_PER_LONG)
#define BITS_PER_BYTE 8
#define BITS_TO_LONGS(nr) DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))

static inline int constant_test_bit(unsigned int nr, const volatile unsigned long *addr)
{
    return ((1UL << (nr % BITS_PER_LONG)) & (addr[nr / BITS_PER_LONG])) != 0;
}

static inline int variable_test_bit(int nr, volatile const unsigned long *addr)
{
    int oldbit;
	asm volatile("bt %2, %1\n\t"
			     "sbb %0, %0"
				 : "=r"(oldbit)
				 : "m"(*(unsigned long *)addr), "Ir"(nr));
	return oldbit;
}

#define test_bit(nr, addr)     \
	(__builtin_constant_p((nr))    \
	 ? constant_test_bit((nr), (addr))    \
	 : variable_test_bit((nr), (addr)))

/*
#define _atomic_spin_lock_irqsave(l,f) do { local_irq_save(f); } while (0)

#define _atomic_spin_unlock_irqrestore(l, f) do { local_irq_restore(f); } while (0)
*/

/* no-atomic.h */
/* What's the effect of he key word "volatile" here */
/*
static inline void __set_bit(int nr, volatile unsigned long *addr)
{
    unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);
	*p |= mask;
}

static inline void __clear_bit(int nr, volatile unsigned long *addr)
{
    unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);

	*p &= ~mask;
}

static inline int test_bit(int nr, const volatile unsigned long *addr)
{
    return 1UL & (addr[BIT_WORD(nr)] >> (nr & (BITS_PER_LONG - 1)));
}

 end -- non-atomic.h
static inline void set_bit(int nr, volatile unsigned long *addr)
{
    unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);
	unsigned long flags;
	_atomic_spin_lock_irqsave(p, flags);
	*p |= mask;
	_atomic_spin_unlock_irqrestore(p, flags);
}

static inline void clear_bit(int nr, volatile unsigned long *addr)
{
    unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);

	_atomic_spin_lock_irqsave(p, flags);
	*p &= ~mask;
	_atomic_spin_unlock_irqrestore(p, flags);
}

static unsigned long __ffs(unsigned long word)
{
    int num = 0;
	
	if ((word & 0xffff) == 0) {
        num += 16;
		word >>= 16;
	}
	if ((word & 0xff) == 0) {
        num += 8;
		word >>= 8;
	}
	if ((word & 0xf) == 0) {
        num += 4;
		word >>= 4;
	}
	if ((word & 0x3) == 0) {
        num += 2;
		word >>= 2;
	}
	if ((word & 0x1) ==0 )
        num += 1;
	return num;
}

#define ffz(x)  __ffs(~(x))

 asm-generic/bitops/fls.h
static int fls(int x)
{
    int r = 32;

	if (!x)
		return 0;

	if (!(x & 0xffff0000u)) {
        x <<= 16;
		r -= 16;
	}
	if (!(x & 0xff000000u)) {
        x <<= 8;
		r -= 8;
	}
	if (!(x & 0xf0000000u)) {
        x <<= 4;
		r -= 4;
	}
	if (!(x & 0xc0000000u)) {
        x <<= 2;
		r -= 2;
	}
	if (!(x & 0x80000000u)) {
        x <<= 1;
		r -= 1;
	}
	return r;
}
 end -- asm-generic/bitops/fls.h 
*/

/* asm/bitops.h */
static inline int ffs(int x)
{
    int r;

	asm("bsfl %1, %0\n\t"
		"comvzl %2, %0"
		: "=r"(r) : "rm"(x), "r"(-1));

	return r + 1;
}

static inline int fls(int x)
{
    int r;

	asm("bsrl %1, %0\n\t"
		"cmovzl %2, %0"
		: "=&r"(r) : "rm"(x), "rm"(-1));

	return r + 1;
}

static inline unsigned long ffz(unsigned long word)
{
    asm("bsf %1, %0"
			: "=r"(word)
			: "r"(~word));
	return word;
}

/* end -- asm/bitops.h */

/* asm-generic/bitops/fls64.h */

static int fls64(__u64 x)
{
    __u32 h = x >> 32;
	if (h)
		return fls(h) + 32;
	return fls(x);
}

/* end -- asm-generic/bitops/fls64.h */

static inline unsigned fls_long(unsigned long l)
{
	
    if (sizeof(l) == 4)
		return fls(l);
	return fls64(l);
	
}

#endif
