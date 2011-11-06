#include <asm/e820.h>
#include <Xc/kernel.h>
#include <asm/page_types.h>
#include <Xc/memblock.h>

struct e820map e820;

#define MAX_ARCH_PFN (1ULL << (32 - PAGE_SHIFT))

int sanitize_e820_map(struct e820entry *biosmap, int max_nr_map,
			     u32 *pnr_map)
{
	struct change_member {
		struct e820entry *pbios; /* pointer to original bios entry */
		unsigned long long addr; /* address for this change point */
	};
	static struct change_member change_point_list[2*E820MAX];
	static struct change_member *change_point[2*E820MAX];
	static struct e820entry *overlap_list[E820MAX];
	static struct e820entry new_bios[E820MAX];
	struct change_member *change_tmp;
	unsigned long current_type, last_type;
	unsigned long long last_addr;
	int chgidx, still_changing;
	int overlap_entries;
	int new_bios_entry;
	int old_nr, new_nr, chg_nr;
	int i;

	/* if there's only one memory region, don't bother */
	if (*pnr_map < 2)
		return -1;

	old_nr = *pnr_map;
	if (old_nr > max_nr_map)
		return -1;

	/* bail out if we find any unreasonable addresses in bios map */
	for (i = 0; i < old_nr; i++)
		if (biosmap[i].addr + biosmap[i].size < biosmap[i].addr)
			return -1;

	/* create pointers for initial change-point information (for sorting) */
	for (i = 0; i < 2 * old_nr; i++)
		change_point[i] = &change_point_list[i];

	/* record all known change-points (starting and ending addresses),
	   omitting those that are for empty memory regions */
	chgidx = 0;
	for (i = 0; i < old_nr; i++)	{
		if (biosmap[i].size != 0) {
			change_point[chgidx]->addr = biosmap[i].addr;
			change_point[chgidx++]->pbios = &biosmap[i];
			change_point[chgidx]->addr = biosmap[i].addr +
				biosmap[i].size;
			change_point[chgidx++]->pbios = &biosmap[i];
		}
	}
	chg_nr = chgidx;

	/* sort change-point list by memory addresses (low -> high) */
	still_changing = 1;
	while (still_changing)	{
		still_changing = 0;
		for (i = 1; i < chg_nr; i++)  {
			unsigned long long curaddr, lastaddr;
			unsigned long long curpbaddr, lastpbaddr;

			curaddr = change_point[i]->addr;
			lastaddr = change_point[i - 1]->addr;
			curpbaddr = change_point[i]->pbios->addr;
			lastpbaddr = change_point[i - 1]->pbios->addr;

			/*
			 * swap entries, when:
			 *
			 * curaddr > lastaddr or
			 * curaddr == lastaddr and curaddr == curpbaddr and
			 * lastaddr != lastpbaddr
			 */
			if (curaddr < lastaddr ||
			    (curaddr == lastaddr && curaddr == curpbaddr &&
			     lastaddr != lastpbaddr)) {
				change_tmp = change_point[i];
				change_point[i] = change_point[i-1];
				change_point[i-1] = change_tmp;
				still_changing = 1;
			}
		}
	}

	/* create a new bios memory map, removing overlaps */
	overlap_entries = 0;	 /* number of entries in the overlap table */
	new_bios_entry = 0;	 /* index for creating new bios map entries */
	last_type = 0;		 /* start with undefined memory type */
	last_addr = 0;		 /* start with 0 as last starting address */

	/* loop through change-points, determining affect on the new bios map */
	for (chgidx = 0; chgidx < chg_nr; chgidx++) {
		/* keep track of all overlapping bios entries */
		if (change_point[chgidx]->addr ==
		    change_point[chgidx]->pbios->addr) {
			/*
			 * add map entry to overlap list (> 1 entry
			 * implies an overlap)
			 */
			overlap_list[overlap_entries++] =
				change_point[chgidx]->pbios;
		} else {
			/*
			 * remove entry from list (order independent,
			 * so swap with last)
			 */
			for (i = 0; i < overlap_entries; i++) {
				if (overlap_list[i] ==
				    change_point[chgidx]->pbios)
					overlap_list[i] =
						overlap_list[overlap_entries-1];
			}
			overlap_entries--;
		}
		/*
		 * if there are overlapping entries, decide which
		 * "type" to use (larger value takes precedence --
		 * 1=usable, 2,3,4,4+=unusable)
		 */
		current_type = 0;
		for (i = 0; i < overlap_entries; i++)
			if (overlap_list[i]->type > current_type)
				current_type = overlap_list[i]->type;
		/*
		 * continue building up new bios map based on this
		 * information
		 */
		if (current_type != last_type)	{
			if (last_type != 0)	 {
				new_bios[new_bios_entry].size =
					change_point[chgidx]->addr - last_addr;
				/*
				 * move forward only if the new size
				 * was non-zero
				 */
				if (new_bios[new_bios_entry].size != 0)
					/*
					 * no more space left for new
					 * bios entries ?
					 */
					if (++new_bios_entry >= max_nr_map)
						break;
			}
			if (current_type != 0)	{
				new_bios[new_bios_entry].addr =
					change_point[chgidx]->addr;
				new_bios[new_bios_entry].type = current_type;
				last_addr = change_point[chgidx]->addr;
			}
			last_type = current_type;
		}
	}
	/* retain count for new bios entries */
	new_nr = new_bios_entry;

	/* copy new bios mapping into original location */
	memcpy(biosmap, new_bios, new_nr * sizeof(struct e820entry));
	*pnr_map = new_nr;

	return 0;
}

void memblock_x86_fill(void)
{
    int i;
	u64 end;
    
	/* By harvey, I don't resize the regions array when the array is out of space. */

	for (i = 0; i < e820.nr_map; i++) {
        struct e820entry *ei = &e820.map[i];

		end = ei->addr + ei->size;
		if (end != (resource_size_t)end)    /* What does this mean? */
			continue;

		if (ei->type != E820_RAM && ei->type != E820_RESERVED_KERN)
			continue;

		if (memblock_add(ei->addr, ei->size) == -1)
			panic("memblock overflows!");
	}

	memblock_analyze();
}

/* Find the largest page frame number we have available*/
static unsigned long e820_end_pfn(unsigned long limit_pfn, unsigned type)
{
    int i;
	unsigned long last_pfn = 0;
	unsigned long max_arch_pfn = MAX_ARCH_PFN;

	for (i = 0; i < e820.nr_map; i++) {
        struct e820entry *ei = &e820.map[i];
		unsigned long start_pfn;
		unsigned long end_pfn;

		if (ei->type != type)
			continue;
		start_pfn = ei->addr >> PAGE_SHIFT;
		end_pfn = (ei->addr + ei->size) >> PAGE_SHIFT;

		if (start_pfn >= limit_pfn)
			continue;
		if (end_pfn > limit_pfn) {
            last_pfn = limit_pfn;
			break;
		}
		if (end_pfn > last_pfn)
			last_pfn = end_pfn;
	}

	if (last_pfn > max_arch_pfn)
		last_pfn = max_arch_pfn;

	return last_pfn;
}

unsigned long e820_end_of_ram_pfn(void)
{
    return e820_end_pfn(MAX_ARCH_PFN, E820_RAM);
}
