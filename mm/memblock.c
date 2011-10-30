#include <Xc/memblock.h>
#include <Xc/mm.h>
#include <asm/page.h>
#include <asm/e820.h>
#include <Xc/linkage.h>
#include <Xc/kernel.h>

struct memblock memblock;

static struct memblock_region memblock_memory_init_regions[INIT_MEMBLOCK_REGIONS + 1];
static struct memblock_region memblock_reserved_init_regions[INIT_MEMBLOCK_REGIONS + 1];

#define RED_INACTIVE 0x09f911029d74e35bULL
#define RED_ACTIVE   0xd84156c5635688c0ULL

void memblock_init(void)
{
    static int init_done;
	if (init_done)
		return;

	init_done = 1;

	memblock.memory.regions = memblock_memory_init_regions;
	memblock.memory.max = INIT_MEMBLOCK_REGIONS;
	memblock.reserved.regions = memblock_reserved_init_regions;
	memblock.reserved.max = INIT_MEMBLOCK_REGIONS;

	memblock.memory.regions[INIT_MEMBLOCK_REGIONS].base = (phys_addr_t)RED_INACTIVE;
	memblock.reserved.regions[INIT_MEMBLOCK_REGIONS].base = (phys_addr_t)RED_INACTIVE;

	memblock.memory.regions[0].base = 0;
	memblock.memory.regions[0].size = 0;
	memblock.memory.cnt = 1;

	memblock.reserved.regions[0].base = 0;
	memblock.reserved.regions[0].size = 0;
	memblock.reserved.cnt = 1;

	memblock.current_limit = MEMBLOCK_ALLOC_ANYWHERE;
}

static void memblock_remove_region(struct memblock_type *type, unsigned long r)
{
    unsigned long i;

    for (i = r; i < type->cnt - 1; i++) {
        type->regions[i].base = type->regions[i + 1].base;  
        type->regions[i].size = type->regions[i + 1].size;
	}
	type->cnt--;

	if (type->cnt == 0) {
        type->cnt = 1;
		type->regions[0].base = type->regions[0].size = 0;
	}
}

extern int __weak memblock_memory_can_coalesce(phys_addr_t addr1, phys_addr_t size1, phys_addr_t addr2, phys_addr_t size2)
{
    return 1;
}

void memblock_analyze(void)
{
	int i;
	memblock.memory_size = 0;
    for (i = 0; i < memblock.memory.cnt; i++)
	    memblock.memory_size += memblock.memory.regions[i].size;
}

static long memblock_add_region(struct memblock_type *type, phys_addr_t base, phys_addr_t size)
{
    phys_addr_t end = base + size;
	int i;

	for (i = 0; i < type->cnt; i++)
	{
        struct memblock_region *rgn = &type->regions[i];
		phys_addr_t rend = rgn->base + rgn->size;

		if (rgn->base > end || rgn->size ==0)
			break;

		if (rgn->base <= base && rend >= end)
			return 0;

		if (base < rgn->base && end >= rgn->base) {
            if (!memblock_memory_can_coalesce(base, size, rgn->base, rgn->size)) {
                goto new_block;
			}

			rgn->base = base;
			rgn->size = rend - base;

			if (rend >= end)
				return 0;

			base = rend;
			size = end - base;
		}

		if (base <= rend && end >= rend) {
            if (!memblock_memory_can_coalesce(rgn->base, rgn->size, base, size)) {
                goto new_block;
			}

			size += (base - rgn->base);
			base = rgn->base;
			memblock_remove_region(type, i--);
		}
	}

	if ((type->cnt == 1) && (type->regions[0].size == 0)) {
        type->regions[0].base = base;
		type->regions[0].size = size;
		return 0;
	}

new_block:
	
	if (type->cnt >= type->max)
		return -1;

	for (i = type->cnt - 1; i >= 0; i--) {
        if (base < type->regions[i].base) {
            type->regions[i + 1].base = type->regions[i].base;
			type->regions[i + 1].size = type->regions[i].size;
		} else {
            type->regions[i + 1].base = base;
			type->regions[i + 1].size = size;
			break;
		}
	}

	if (base < type->regions[0].base) {
        type->regions[0].base = base;
		type->regions[0].size = size;
	}

	type->cnt++;

	if (type->cnt >= type->max)
	{
        /* TODO: Resize the regions array */
	}
	
	return 0;
}

long memblock_add(phys_addr_t base, phys_addr_t size)
{
    return memblock_add_region(&memblock.memory, base, size);
}
/*
u64 memblock_find_in_range(u64 start, u64 end, u64 size, u64 align)
{
    return memblock_find_base(size, align ,start, end);
}
*/

static unsigned long memblock_addrs_overlap(phys_addr_t base1, phys_addr_t size1, phys_addr_t base2, phys_addr_t size2)
{
    return ((base1 < (base2 + size2)) &&( base2 < (base1 + size1)));
}

static phys_addr_t memblock_align_down(phys_addr_t addr, phys_addr_t size)
{
    return addr & ~(size - 1);
}

long memblock_overlaps_region(struct memblock_type *type, phys_addr_t base, phys_addr_t size)
{
    unsigned long i;
	for (i = 0; i < type->cnt; i++) {
        phys_addr_t rgnbase = type->regions[i].base;
		phys_addr_t rgnsize = type->regions[i].size;
		if (memblock_addrs_overlap(base, size, rgnbase, rgnsize))
				break;
	}
	return (i < type->cnt) ? i : -1;
}

static phys_addr_t memblock_find_region(phys_addr_t start, phys_addr_t end, phys_addr_t size, phys_addr_t align)
{
    phys_addr_t base, res_base;
	long j;

	if (end < size)
		return MEMBLOCK_ERROR;
	base = memblock_align_down((end - size), align);

	if(start == 0)
		start = PAGE_SIZE;

	while (start <= base) {
       j = memblock_overlaps_region(&memblock.reserved, base, size);
	   if (j < 0)
		   return base;
	   res_base = memblock.reserved.regions[j].base;
	   if (res_base < size)
		   break;
	   base = memblock_align_down(res_base - size, align);
	}

	return MEMBLOCK_ERROR;
}

static phys_addr_t memblock_find_base(phys_addr_t size, phys_addr_t align, phys_addr_t start, phys_addr_t end)
{
    long i;

	if (end == MEMBLOCK_ALLOC_ACCESSIBLE)
		end = memblock.current_limit;

	for (i = memblock.memory.cnt - 1; i >= 0; i--) {
        phys_addr_t memblockbase = memblock.memory.regions[i].base;
		phys_addr_t memblocksize = memblock.memory.regions[i].size;
		phys_addr_t bottom, top, found;

		if (memblocksize < size)
			continue;

		if ((memblockbase + memblocksize) <= start)
			break;

		bottom = max(memblockbase, start);
		top = min(memblockbase + memblocksize, end);

		if (bottom >= top)
			continue;

		found = memblock_find_region(bottom, top, size, align);
		if (found != MEMBLOCK_ERROR)
			return found;
	}

	return MEMBLOCK_ERROR;
}

u64 memblock_find_in_range(u64 start, u64 end, u64 size, u64 align)
{
    return memblock_find_base(size, align, start, end);
}

long memblock_reserve(phys_addr_t base, phys_addr_t size)
{
    struct memblock_type *_rgn = &memblock.reserved;

	return memblock_add_region(_rgn, base, size);
}

void memblock_x86_reserve_range(u64 start, u64 end, char *name)
{
    if (start == end)
		return;

	memblock_reserve(start, end - start);
}


static int memblock_x86_find_active_region(const struct memblock_region *ei, unsigned long start_pfn,
		unsigned long last_pfn, unsigned long *ei_startpfn, unsigned long *ei_endpfn)
{
    u64 align = PAGE_SIZE;

	*ei_startpfn = round_up(ei->base, align) >> PAGE_SHIFT;
	*ei_endpfn = round_down(ei->base + ei->size, align) >> PAGE_SHIFT;

	if (*ei_startpfn >= *ei_endpfn)
		return 0;

	if (*ei_endpfn <= start_pfn || *ei_startpfn >= last_pfn)
		return 0;

	if (*ei_startpfn < start_pfn)
		*ei_startpfn = start_pfn;
	if (*ei_endpfn > last_pfn)
		*ei_endpfn = last_pfn;

	return 1;
}

void memblock_x86_register_active_regions(int nid, unsigned long start_pfn, unsigned long last_pfn)
{
    unsigned long ei_startpfn;
	unsigned long ei_endpfn;
	struct memblock_region *r;

	for_each_memblock(memory, r)
		if (memblock_x86_find_active_region(r, start_pfn, last_pfn, &ei_startpfn, &ei_endpfn))
			add_active_range(nid, ei_startpfn, ei_endpfn);
}
