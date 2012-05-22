#ifndef _XC_PREFETCH_H
#define _XC_PREFETCH_H

#define prefetch(x)  __builtin_prefetch(x)
#define prefetchw(x)  __builtin_prefetch(x, 1)

#endif
