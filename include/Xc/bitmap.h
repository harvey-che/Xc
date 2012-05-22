#ifndef _XC_BITMAP_H
#define _XC_BITMAP_H
#include <Xc/string.h>
#include <Xc/bitops.h>
#include <Xc/kernel.h>


#define BITMAP_LAST_WORD_MASK(nbits)    \
	(              \
	    ((nbits) % BITS_PER_LONG) ?     \
				   (1UL << ((nbits) % BITS_PER_LONG))-1 : ~0UL    \
	)

#define small_const_nbits(nbits)    \
	(__builtin_constant_p(nbits) && (nbits) <= BITS_PER_LONG)

static inline void bitmap_zero(unsigned long *dst, int nbits)
{
    if (small_const_nbits(nbits))
		*dst = 0UL;
	else {
        int len = BITS_TO_LONGS(nbits) * sizeof(unsigned long);
		memset(dst ,0 , len);
	}
}

#endif
