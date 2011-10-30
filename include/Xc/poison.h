#ifndef _XC_POISON_H
#define _XC_POISON_H

#define SLUB_RED_INACTIVE 0xbb
#define SLUB_RED_ACTIVE   0xcc

#define POISON_INUSE 0x5a
#define POISON_FREE  0x6b
#define POISON_END   0xa5

#endif
