#include <Xc/mm.h>
#include <Xc/bootmem.h>
#include <Xc/memblock.h>
#include <Xc/slab.h>
#include <Xc/string.h>

u32 max_pfn;
u32 max_low_pfn;
u32 min_low_pfn;

struct pglist_data contig_page_data;


static void* __alloc_memory_core_early(int nid, u64 size, u64 align, u64 goal, u64 limit)
{
    void *ptr;
	u64 addr;

	if (limit > memblock.current_limit)
		limit = memblock.current_limit;

	addr = find_memory_core_early(nid, size, align, goal, limit);

	if (addr == MEMBLOCK_ERROR)
		return NULL;

	ptr = phys_to_virt(addr);
	memset(ptr, 0, size);

	memblock_x86_reserve_range(addr, addr + size, "BOOTMEM");

	/* kmemleak_alloc(ptr, size, 0, 0); */
	return ptr;
}


static void *___alloc_bootmem_nopanic(unsigned long size, unsigned long align, unsigned long goal,
                              unsigned long limit)
{
    void *ptr;
    
	if (slab_is_available())
		return kzalloc(size, GFP_NOWAIT);
restart:
	ptr = __alloc_memory_core_early(MAX_NUMNODES, size, align, goal, limit);
	if (ptr)
		return ptr;
	if (goal != 0) {
        goal = 0;
		goto restart;
	}

	return NULL;
}

static void *___alloc_bootmem(unsigned long size, unsigned long align, unsigned long goal,
                              unsigned long limit)
{
    void *mem = ___alloc_bootmem_nopanic(size, align, goal, limit);
	if (mem)
        return mem;
	panic("Out of memory");
	return NULL;
}

void *__alloc_bootmem(unsigned long size, unsigned long align, unsigned long goal)
{
    unsigned long limit = -1UL;
	return ___alloc_bootmem(size, align, goal, limit);
}

void * __alloc_bootmem_nopanic(unsigned long size, unsigned long align, unsigned long goal)
{
    unsigned long limit = -1UL;
	return ___alloc_bootmem_nopanic(size, align, goal, limit);
}

void * __alloc_bootmem_node_nopanic(pg_data_t *pgdat, unsigned long size, unsigned long align, 
		unsigned long goal)
{
	void *ptr;

    if (slab_is_available())
		return kzalloc_node(size, GFP_NOWAIT, pgdat->node_id);

	ptr = __alloc_memory_core_early(pgdat->node_id, size, align, goal, -1ULL);
	
	if (ptr)
		return ptr;

	return __alloc_bootmem_nopanic(size, align, goal);
}
