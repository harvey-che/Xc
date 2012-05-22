#ifndef _XC_BOOTMEM_H
#define _XC_BOOTMEM_H

#include <Xc/types.h>
#include <asm/cache.h>
#include <Xc/mmzone.h>
#include <Xc/init.h>

#define BOOTMEM_LOW_LIMIT 0

extern unsigned long max_pfn;
extern unsigned long max_low_pfn;
extern unsigned long min_low_pfn;

extern struct memblock memblock;

#define alloc_bootmem_nopanic(x)   \
	__alloc_bootmem_nopanic(x, SMP_CACHE_BYTES, BOOTMEM_LOW_LIMIT)

#define alloc_bootmem_pages(x)   \
	__alloc_bootmem(x, PAGE_SIZE, BOOTMEM_LOW_LIMIT)

#define alloc_bootmem_node_nopanic(pgdat, x)   \
	__alloc_bootmem_node_nopanic(pgdat, x, SMP_CACHE_BYTES, BOOTMEM_LOW_LIMIT)

extern void *__alloc_bootmem(unsigned long size, unsigned long align, unsigned long goal);

extern void *__alloc_bootmem_nopanic(unsigned long size, unsigned long align, unsigned long goal);

extern void *__alloc_bootmem_node_nopanic(pg_data_t *pgdat, unsigned long size, unsigned long align,
		unsigned long goal);

unsigned long free_all_memory_core_early(int nodeid);
extern unsigned long free_all_bootmem(void);

static inline void *alloc_remap(int nid, unsigned long size)
{
    return NULL;
}

#define HASH_EARLY    0x00000001
#define HASH_SMALL    0x00000002

void *__init alloc_large_system_hash(const char *tablename, unsigned long bucketsize, 
		                             unsigned long numentries, int scale, int flags, 
									 unsigned int *_hash_shift, unsigned int *_hash_mask, 
									 unsigned long limit);
#endif
