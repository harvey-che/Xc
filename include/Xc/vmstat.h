#ifndef _XC_VMSTAT_H
#define _XC_VMSTAT_H

#include <Xc/atomic.h>
#include <Xc/mmzone.h>

static inline void zone_page_state_add(long x, struct zone *zone, enum zone_stat_item item)
{
    atomic_long_add(x, &zone->vm_stat[item]);
	/* atomic_long_add(x, &vm_stat[item]); */
}

static inline void __mod_zone_page_state(struct zone *zone, enum zone_stat_item item, int delta)
{
    zone_page_state_add(delta, zone, item);
}

#define mod_zone_page_state __mod_zone_page_state


static inline unsigned long zone_page_state(struct zone *zone, enum zone_stat_item item)
{
    long x = atomic_long_read(&zone->vm_stat[item]);
	return x;
}

#define zone_statistics(_zl, _z, gfp) do {} while (0)
#endif
