#ifndef _XC_VMSTAT_H
#define _XC_VMSTAT_H

#include <Xc/atomic.h>
#include <Xc/mmzone.h>
#include <Xc/mm.h>

extern atomic_long_t vm_stat[NR_VM_ZONE_STAT_ITEMS];

static inline void zone_page_state_add(long x, struct zone *zone, enum zone_stat_item item)
{
    atomic_long_add(x, &zone->vm_stat[item]);
	/* atomic_long_add(x, &vm_stat[item]); */
}

static inline unsigned long zone_page_state(struct zone *zone, enum zone_stat_item item)
{
    long x = atomic_long_read(&zone->vm_stat[item]);
	return x;
}

#ifdef CONFIG_SMP

#else

static inline void __mod_zone_page_state(struct zone *zone, enum zone_stat_item item, int delta)
{
    zone_page_state_add(delta, zone, item);
}

#define mod_zone_page_state __mod_zone_page_state

static inline void __dec_zone_state(struct zone *zone, enum zone_stat_item item)
{
	atomic_long_dec(&zone->vm_stat[item]);
	atomic_long_dec(&vm_stat[item]);
}

static inline void __dec_zone_page_state(struct page *page, enum zone_stat_item item)
{
    __dec_zone_state(page_zone(page), item);
}

#endif /* CONFIG_SMP */

#ifdef CONFIG_NUMA

#else

#define zone_statistics(_zl, _z, gfp) do {} while (0)

#endif

#endif
