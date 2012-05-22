#ifndef _XC_MEMBLOCK_H
#define _XC_MEMBLOCK_H

#ifdef __KERNEL__

#include <Xc/types.h>
#include <Xc/range.h>

#define MEMBLOCK_ERROR 0

#define INIT_MEMBLOCK_REGIONS   128

#define MEMBLOCK_ALLOC_ANYWHERE (~(phys_addr_t)0)
#define MEMBLOCK_ALLOC_ACCESSIBLE 0


struct memblock_region {
    phys_addr_t base;
	phys_addr_t size;
};

struct memblock_type {
    unsigned long cnt;
	unsigned long max;
	struct memblock_region *regions;
};

struct memblock {
    phys_addr_t current_limit;
	phys_addr_t memory_size;
	struct memblock_type memory;
	struct memblock_type reserved;
};

extern struct memblock memblock;

#define for_each_memblock(memblock_type, region)    \
	for (region = memblock.memblock_type.regions;     \
		 region < (memblock.memblock_type.regions + memblock.memblock_type.cnt);   \
		 region++)

u64 memblock_find_in_range(u64 start, u64 end, u64 size, u64 align);
void memblock_x86_reserve_range(u64 start, u64 end, char *name);
void memblock_x86_register_active_regions(int nid, unsigned long start_pfn, 
		                                  unsigned long last_pfn);

extern void memblock_init(void);
extern void memblock_analyze(void);
extern long memblock_add(phys_addr_t base, phys_addr_t size);

int get_free_all_memory_range(struct range **rangep, int nodeid);

#endif

#endif
