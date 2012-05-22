#ifndef _XC_KERNEL_UTILITIES_H
#define _XC_KERNEL_UTILITIES_H

#include <asm/page.h>
#include <Xc/types.h>
#include <Xc/bitops.h>
#include <stddef.h>

int printk(const char *format, ...);
void panic(const char *format, ...);
char *itoah(char *str, unsigned long num);
int early_print_str(const char *str);
int early_painc(const char *str);

#define __round_mask(x, y) ((__typeof__(x))((y) - 1))
#define round_up(x, y) ((((x) - 1) | __round_mask(x, y)) + 1)
#define round_down(x, y) ((x) & ~__round_mask(x, y))

#define DIV_ROUND_UP(n, d) (((n) + (d) -1) / (d))

#define roundup(x, y) (                 \
{                                       \
	const typeof(y) __y = y;            \
	(((x) + (__y - 1)) / __y) * __y;    \
}                                       \
)

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof((arr)[0]))

#define max(x, y)  ({    \
		typeof(x) _max1 = (x);    \
		typeof(y) _max2 = (y);    \
		(void) (&_max1 == &_max2);    \
		_max1 > _max2 ? _max1 : _max2;})

#define min(x, y)  ({    \
		typeof(x) _min1 = (x);    \
		typeof(y) _min2 = (y);    \
		(void) (&_min1 == &_min2);    \
		_min1 < _min2 ? _min1 : _min2;})

#define ULONG_MAX (~0UL)


#define ALIGN(x, a) (((x) + ((typeof(x))(a) - 1)) & ~((typeof(x))(a) - 1))

extern void sort(void *base, size_t num, size_t size, int (*cmp_func)(const void *, const void *), 
		void (*swap_func)(void *, void *, int size));

#define might_resched() _cond_resched()

#define might_sleep() do { might_resched(); } while (0)

#define might_sleep_if(cond) do { if (cond) might_sleep(); } while (0)

/* log2.h */

extern int ____ilog2_NaN(void);

static int __ilog2_u32(u32 n)
{
    return fls(n) - 1;
}

static int __ilog2_u64(u64 n)
{
    return  fls64(n) - 1;
}

#define ilog2(n)				\
(						\
	__builtin_constant_p(n) ? (		\
		(n) < 1 ? ____ilog2_NaN() :	\
		(n) & (1ULL << 63) ? 63 :	\
		(n) & (1ULL << 62) ? 62 :	\
		(n) & (1ULL << 61) ? 61 :	\
		(n) & (1ULL << 60) ? 60 :	\
		(n) & (1ULL << 59) ? 59 :	\
		(n) & (1ULL << 58) ? 58 :	\
		(n) & (1ULL << 57) ? 57 :	\
		(n) & (1ULL << 56) ? 56 :	\
		(n) & (1ULL << 55) ? 55 :	\
		(n) & (1ULL << 54) ? 54 :	\
		(n) & (1ULL << 53) ? 53 :	\
		(n) & (1ULL << 52) ? 52 :	\
		(n) & (1ULL << 51) ? 51 :	\
		(n) & (1ULL << 50) ? 50 :	\
		(n) & (1ULL << 49) ? 49 :	\
		(n) & (1ULL << 48) ? 48 :	\
		(n) & (1ULL << 47) ? 47 :	\
		(n) & (1ULL << 46) ? 46 :	\
		(n) & (1ULL << 45) ? 45 :	\
		(n) & (1ULL << 44) ? 44 :	\
		(n) & (1ULL << 43) ? 43 :	\
		(n) & (1ULL << 42) ? 42 :	\
		(n) & (1ULL << 41) ? 41 :	\
		(n) & (1ULL << 40) ? 40 :	\
		(n) & (1ULL << 39) ? 39 :	\
		(n) & (1ULL << 38) ? 38 :	\
		(n) & (1ULL << 37) ? 37 :	\
		(n) & (1ULL << 36) ? 36 :	\
		(n) & (1ULL << 35) ? 35 :	\
		(n) & (1ULL << 34) ? 34 :	\
		(n) & (1ULL << 33) ? 33 :	\
		(n) & (1ULL << 32) ? 32 :	\
		(n) & (1ULL << 31) ? 31 :	\
		(n) & (1ULL << 30) ? 30 :	\
		(n) & (1ULL << 29) ? 29 :	\
		(n) & (1ULL << 28) ? 28 :	\
		(n) & (1ULL << 27) ? 27 :	\
		(n) & (1ULL << 26) ? 26 :	\
		(n) & (1ULL << 25) ? 25 :	\
		(n) & (1ULL << 24) ? 24 :	\
		(n) & (1ULL << 23) ? 23 :	\
		(n) & (1ULL << 22) ? 22 :	\
		(n) & (1ULL << 21) ? 21 :	\
		(n) & (1ULL << 20) ? 20 :	\
		(n) & (1ULL << 19) ? 19 :	\
		(n) & (1ULL << 18) ? 18 :	\
		(n) & (1ULL << 17) ? 17 :	\
		(n) & (1ULL << 16) ? 16 :	\
		(n) & (1ULL << 15) ? 15 :	\
		(n) & (1ULL << 14) ? 14 :	\
		(n) & (1ULL << 13) ? 13 :	\
		(n) & (1ULL << 12) ? 12 :	\
		(n) & (1ULL << 11) ? 11 :	\
		(n) & (1ULL << 10) ? 10 :	\
		(n) & (1ULL <<  9) ?  9 :	\
		(n) & (1ULL <<  8) ?  8 :	\
		(n) & (1ULL <<  7) ?  7 :	\
		(n) & (1ULL <<  6) ?  6 :	\
		(n) & (1ULL <<  5) ?  5 :	\
		(n) & (1ULL <<  4) ?  4 :	\
		(n) & (1ULL <<  3) ?  3 :	\
		(n) & (1ULL <<  2) ?  2 :	\
		(n) & (1ULL <<  1) ?  1 :	\
		(n) & (1ULL <<  0) ?  0 :	\
		____ilog2_NaN()			\
				   ) :		\
	(sizeof(n) <= 4) ?			\
	__ilog2_u32(n) :			\
	__ilog2_u64(n)				\
 )

static inline unsigned long __roundup_pow_of_two(unsigned long n)
{
    return 1UL << fls_long(n - 1);
}

#define roundup_pow_of_two(n)     \
	(                             \
	    __builtin_constant_p(n) ? (    \
		    (n == 1) ? 1 :        \
			(1UL << (ilog2((n) - 1) + 1))    \
			                      ) :    \
			__roundup_pow_of_two(n)    \
	)

/* end -- log2.h */

#define NUMA_BUILD 0

static inline int get_order(unsigned long size)
{
    int order;
	size = (size - 1) >> (PAGE_SHIFT - 1);
	order = -1;
	do {
		size >>= 1;
		order++;
	} while (size);
	return order;
}

#define container_of(ptr, type, member) ({    \
		const typeof(((type *)0)->member) * __mptr = (ptr);    \
		(type *)((char *)__mptr - offsetof(type, member));})

#define _RET_IP_ (unsigned long)__builtin_return_address(0)

#define printk_ratelimit() (0) 
#endif
