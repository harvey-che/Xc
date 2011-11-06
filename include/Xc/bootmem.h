#ifndef _XC_BOOTMEM_H
#define _XC_BOOTMEM_H

#include <Xc/types.h>
#include <asm/cache.h>
#include <Xc/mmzone.h>

#define BOOTMEM_LOW_LIMIT 0

extern unsigned long max_pfn;
extern unsigned long max_low_pfn;
extern unsigned long min_low_pfn;

extern struct memblock memblock;

#define alloc_bootmem_pages(x)   \
	__alloc_bootmem(x, PAGE_SIZE, BOOTMEM_LOW_LIMIT)

#define alloc_bootmem_node_nopanic(pgdat, x)   \
	__alloc_bootmem_node_nopanic(pgdat, x, SMP_CACHE_BYTES, BOOTMEM_LOW_LIMIT)

extern void *__alloc_bootmem(unsigned long size, unsigned long align, unsigned long goal);

extern void *__alloc_bootmem_node_nopanic(pg_data_t *pgdat, unsigned long size, unsigned long align,
		unsigned long goal);

unsigned long free_all_memory_core_early(int nodeid);
extern unsigned long free_all_bootmem(void);

static inline void *alloc_remap(int nid, unsigned long size)
{
    return NULL;
}
#endif
