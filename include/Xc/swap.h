#ifndef _XC_SWAP_H
#define _XC_SWAP_H
#include <Xc/mmzone.h>

/* CONFIG_NUMA is not defined */
#define zone_reclaim_mode 0

struct reclaim_state {
    unsigned long reclaimed_slab;
};

static inline int zone_reclaim(struct zone *z, gfp_t mask, unsigned int order)
{
    return 0;
}

extern unsigned long totalram_pages;
extern unsigned long totalreserve_pages;

#endif
