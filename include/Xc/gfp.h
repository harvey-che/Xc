#ifndef _XC_GFP_H
#define _XC_GFP_H

#include <Xc/types.h>
#include <Xc/mmzone.h>
#include <stddef.h>
#include <Xc/kernel.h>


#define ___GFP_DMA     0x01u
#define ___GFP_HIGHMEM 0x02u
#define ___GFP_DMA32   0x04u
#define ___GFP_MOVABLE 0x08u
#define ___GFP_WAIT    0x10u
#define ___GFP_HIGH    0x20u
#define ___GFP_IO      0x40u
#define ___GFP_FS      0x80u
#define ___GFP_COLD    0x100u
#define ___GFP_NOWARN  0x200u
#define ___GFP_REPEAT  0x400u
#define ___GFP_NOFAIL  0x800u
#define ___GFP_NORETRY 0x1000u
#define ___GFP_COMP    0x4000u
#define ___GFP_ZERO    0x8000u
#define ___GFP_NOMEMALLOC 0x10000u
#define ___GFP_HARDWALL 0x20000u
#define ___GFP_THISNODE 0x40000u
#define ___GFP_RECLAIMABLE 0x80000u
#define ___GFP_NOTRACK  0
#define ___GFP_NO_KSWAPD 0x400000u
#define ___GFP_OTHER_NODE 0x800000u

#define __GFP_WAIT ((gfp_t)___GFP_WAIT)
#define __GFP_HIGH ((gfp_t)___GFP_HIGH)
#define __GFP_IO   ((gfp_t)___GFP_IO)
#define __GFP_FS   ((gfp_t)___GFP_FS)
#define __GFP_COLD   ((gfp_t)___GFP_COLD)
#define __GFP_NOWARN   ((gfp_t)___GFP_NOWARN)
#define __GFP_REPEAT   ((gfp_t)___GFP_REPEAT)
#define __GFP_NOFAIL   ((gfp_t)___GFP_NOFAIL)
#define __GFP_NORETRY   ((gfp_t)___GFP_NORETRY)
#define __GFP_COMP   ((gfp_t)___GFP_COMP)
#define __GFP_ZERO   ((gfp_t)___GFP_ZERO)
#define __GFP_NOMEMALLOC   ((gfp_t)___GFP_NOMEMALLOC)
#define __GFP_HARDWALL   ((gfp_t)___GFP_HARDWALL)
#define __GFP_THISNODE   ((gfp_t)___GFP_THISNODE)
#define __GFP_RECLAMIABLE   ((gfp_t)___GFP_RECLAIMABLE)
#define __GFP_NOTRACK   ((gfp_t)___GFP_NOTRACK)
#define __GFP_NO_KWAPD   ((gfp_t)___GFP_NO_KSWAPD)
#define __GFP_OTHER_NODE   ((gfp_t)___GFP_OTHER_NODE)

#define __GFP_DMA   ((gfp_t)___GFP_DMA)
#define __GFP_HIGHMEM ((gfp_t)___GFP_HIGHMEM)
#define __GFP_DMA32 ((gfp_t)___GFP_DMA32)
#define __GFP_MOVABLE ((gfp_t)___GFP_MOVABLE)
#define __GFP_ZERO  ((gfp_t)___GFP_ZERO)
#define __GFP_THISNODE ((gfp_t)___GFP_THISNODE)
#define __GFP_RECLAIMABLE ((gfp_t)___GFP_RECLAIMABLE)
#define GFP_ZONEMASK (__GFP_DMA | __GFP_HIGHMEM | __GFP_DMA32 | __GFP_MOVABLE)

#define __GFP_BITS_SHIFT 24
#define __GFP_BITS_MASK ((gfp_t)((1 << __GFP_BITS_SHIFT) - 1))

#define GFP_NOWAIT (GFP_ATOMIC & ~__GFP_HIGH)
#define GFP_ATOMIC (__GFP_HIGH)
#define GFP_KERNEL (__GFP_WAIT | __GFP_IO | __GFP_FS)

#define GFP_RECLAIM_MASK (__GFP_WAIT | __GFP_HIGH | __GFP_IO | __GFP_FS | \
		__GFP_NOWARN | __GFP_REPEAT | __GFP_NOFAIL | __GFP_NORETRY | __GFP_NOMEMALLOC)

#define GFP_BOOT_MASK (__GFP_BITS_MASK & ~(__GFP_WAIT | __GFP_IO || __GFP_FS))

#define GFP_CONSTRAINT_MASK (__GFP_HARDWALL | __GFP_THISNODE)

extern gfp_t gfp_allowed_mask;


#define OPT_ZONE_HIGHMEM ZONE_HIGHMEM
#define OPT_ZONE_DMA ZONE_DMA
#define OPT_ZONE_DMA32 ZONE_NORMAL

#define GFP_ZONE_TABLE (   \
		(ZONE_NORMAL << 0 * ZONES_SHIFT)    \
		| (OPT_ZONE_DMA << ___GFP_DMA * ZONES_SHIFT)    \
		| (OPT_ZONE_HIGHMEM << ___GFP_HIGHMEM * ZONES_SHIFT)   \
		| (OPT_ZONE_DMA32 << ___GFP_DMA32 * ZONES_SHIFT)    \
		| (ZONE_NORMAL << ___GFP_MOVABLE * ZONES_SHIFT)    \
		| (OPT_ZONE_DMA << (___GFP_MOVABLE | ___GFP_DMA) * ZONES_SHIFT)   \
		| (ZONE_MOVABLE << (___GFP_MOVABLE | ___GFP_HIGHMEM) * ZONES_SHIFT)   \
		| (OPT_ZONE_DMA32 << (___GFP_MOVABLE | ___GFP_DMA32) * ZONES_SHIFT)   \
		)

static inline enum zone_type gfp_zone(gfp_t flags)
{
    enum zone_type z;
	int bit = (int)(flags & GFP_ZONEMASK);

	z = (GFP_ZONE_TABLE >> (bit * ZONES_SHIFT)) & ((1 << ZONES_SHIFT) - 1);
	return z;
}

static inline int allocflags_to_migratetype(gfp_t gfp_flags)
{
    if (page_group_by_mobility_disabled)
		return MIGRATE_UNMOVABLE;

	return (((gfp_flags & __GFP_MOVABLE) != 0) << 1) | ((gfp_flags & __GFP_RECLAIMABLE) != 0);
}

extern void __free_pages(struct page *page, unsigned int order);
extern void free_pages(unsigned long addr, unsigned int order);

#define __free_page(page) __free_pages((page), 0)
#define free_page(addr) free_pages((addr), 0)

struct page * __alloc_pages_nodemask(gfp_t gfp_mask, unsigned int order, struct zonelist *zonelist,
		nodemask_t *nodemask);

static inline struct page *__alloc_pages(gfp_t gfp_mask, unsigned int order, struct zonelist *zonelist)
{
    return __alloc_pages_nodemask(gfp_mask, order, zonelist, NULL);
}

static inline int gfp_zonelist(gfp_t flags)
{
    if (NUMA_BUILD && (flags & __GFP_THISNODE))
		return 1;
	return 0;
}

static inline struct zonelist *node_zonelist(int nid, gfp_t flags)
{
    return NODE_DATA(nid)->node_zonelists + gfp_zonelist(flags);
}

static inline struct page *alloc_pages_node(int nid, gfp_t gfp_mask, unsigned int order)
{
    if (nid < 0)
		nid = numa_node_id();

	return __alloc_pages(gfp_mask, order, node_zonelist(nid, gfp_mask));
}

#define alloc_pages(gfp_mask, order)    \
	alloc_pages_node(numa_node_id(), gfp_mask, order)

extern unsigned long __get_free_pages(gfp_t gfp_mask, unsigned int order);

static inline struct page *alloc_pages_exact_node(int nid, gfp_t gfp_mask, unsigned int order)
{
	/*
	if (nid < 0 || nid >= MAX_NUMNODES)
		return NULL;
	*/
    return __alloc_pages(gfp_mask, order, node_zonelist(nid, gfp_mask));
}

static inline void arch_alloc_page(struct page *page, int order) {}
#endif
