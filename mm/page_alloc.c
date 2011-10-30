#include <Xc/kernel.h>
#include <Xc/memblock.h>
#include <Xc/mmzone.h>
#include <Xc/bitops.h>
#include <asm/pgtable_types.h>
#include <stddef.h>
#include <Xc/mm.h>
#include <asm/page.h>
#include <Xc/percpu.h>
#include <Xc/wait.h>
#include <Xc/errno.h>
#include <Xc/slab.h>
#include <Xc/bootmem.h>
#include <Xc/vmalloc.h>
#include <Xc/spinlock.h>
#include <Xc/memory_hotplug.h>
#include <Xc/string.h>
#include <Xc/page-flags.h>
#include <Xc/gfp.h>
#include <Xc/nodemask.h>
#include <Xc/swap.h>
#include <Xc/vmstat.h>
#include <Xc/cpuset.h>
#include <Xc/sched.h>
#include "internal.h"

#define MAX_ACTIVE_REGIONS  256

static struct node_active_region early_node_map[MAX_ACTIVE_REGIONS];
static int nr_nodemap_entries;

static unsigned long arch_zone_lowest_possible_pfn[MAX_NR_ZONES];
static unsigned long arch_zone_highest_possible_pfn[MAX_NR_ZONES];
static unsigned long zone_movable_pfn[MAX_NUMNODES];
static unsigned long nr_kernel_pages;
static unsigned long nr_all_pages;
static unsigned long dma_reserve;

int page_group_by_mobility_disabled;
int movable_zone;

gfp_t gfp_allowed_mask = GFP_BOOT_MASK;

static DEFINE_PER_CPU(struct per_cpu_pageset, boot_pageset);

static char *const zone_names[MAX_NR_ZONES] = {
    "DMA",
	"Normal",
	"HighMem",
	"Movable",
};

nodemask_t node_states[NR_NODE_STATES] = {
	[N_POSSIBLE] = NODE_MASK_ALL,
	[N_ONLINE] = { { [0] = 1UL} },
	[N_NORMAL_MEMORY] = { { [0] = 1UL } },
	/* [N_HIGH_MEMORY] = { { [0] = 1UL } }, */
	[N_CPU] = { { [0] = 1UL } },
};

#define ALLOC_WMARK_MIN  WMARK_MIN
#define ALLOC_WMARK_LOW  WMARK_LOW
#define ALLOC_WMARK_HIGH WMAR_HIGH
#define ALLOC_NO_WATERMARKS 0x40

#define ALLOC_WMARK_MASK (ALLOC_NO_WATERMARKS - 1)

#define ALLOC_HARDER 0x10
#define ALLOC_HIGH   0x20
#define ALLOC_CPUSET 0x40

static int fallbacks[MIGRATE_TYPES][MIGRATE_TYPES] = {
    [MIGRATE_UNMOVABLE] = { MIGRATE_RECLAIMABLE, MIGRATE_MOVABLE, MIGRATE_RESERVE },
	[MIGRATE_RECLAIMABLE] = { MIGRATE_UNMOVABLE, MIGRATE_MOVABLE, MIGRATE_RESERVE },
    [MIGRATE_MOVABLE] = { MIGRATE_RECLAIMABLE, MIGRATE_UNMOVABLE, MIGRATE_RESERVE },
	[MIGRATE_RESERVE] = { MIGRATE_RESERVE, MIGRATE_RESERVE, MIGRATE_RESERVE },
};

static int last_active_region_index_in_nid(int nid)
{
   int i;
   for (i = nr_nodemap_entries - 1; i >= 0; i--)
	   if (nid == MAX_NUMNODES || early_node_map[i].nid == nid)
		   return i;

   return -1;
}

static int previous_active_region_indx_in_nid(int index, int nid)
{
    for (index = index - 1; index >= 0; index--)
		if (nid == MAX_NUMNODES || early_node_map[index].nid == nid)
			return index;

	return -1;
}

static int first_active_region_index_in_nid(int nid)
{
    int i;
	for (i = 0; i < nr_nodemap_entries; i++)
		if (nid == MAX_NUMNODES || early_node_map[i].nid == nid)
			return i;
	return -1;
}

static int next_active_region_index_in_nid(int index, int nid)
{
    for (index = index + 1; index < nr_nodemap_entries; index++)
		if (nid == MAX_NUMNODES || early_node_map[index].nid == nid)
			return index;
	return -1;
}

#define for_each_active_range_index_in_nid_reverse(i, nid)   \
	for (i = last_active_region_index_in_nid(nid); i != -1;   \
			i = previous_active_region_indx_in_nid(i, nid))

#define for_each_active_range_index_in_nid(i, nid)    \
	for (i = first_active_region_index_in_nid(nid); i != -1; i = next_active_region_index_in_nid(i, nid))
u64 find_memory_core_early(int nid, u64 size, u64 align, u64 goal, u64 limit)
{
    int i;
	for_each_active_range_index_in_nid_reverse(i, nid) {
        u64 addr;
		u64 ei_start, ei_last;
		u64 final_start, final_end;
		
		ei_last = early_node_map[i].end_pfn;
		ei_last <<= PAGE_SHIFT;
		ei_start = early_node_map[i].start_pfn;
		ei_start <<= PAGE_SHIFT;

		final_start = max(ei_start, goal);
		final_end = min(ei_last, limit);

		if (final_start >= final_end)
			continue;

		addr = memblock_find_in_range(final_start, final_end, size, align);

		if (addr == MEMBLOCK_ERROR)
			continue;

		return addr;
	}

	return MEMBLOCK_ERROR; 
}

void add_active_range(unsigned int nid, unsigned long start_pfn, unsigned long end_pfn)
{
	int i;
    /* mminit_validate_memmodel_limits(&start_pfn, &end_pfn); */

	for (i = 0; i < nr_nodemap_entries; i++) {
        if (early_node_map[i].nid != nid)
			continue;

		if (start_pfn >= early_node_map[i].start_pfn && end_pfn <= early_node_map[0].end_pfn)
			return;

		/* Note! Extending forwardly or backwardly as simply as below may cause overlapping */
		if (start_pfn <= early_node_map[0].end_pfn && end_pfn > early_node_map[i].end_pfn) {
            early_node_map[i].end_pfn = end_pfn;
			return;
		}

		if (start_pfn < early_node_map[i].start_pfn && end_pfn >= early_node_map[i].start_pfn) {
            early_node_map[i].start_pfn = start_pfn;
			return;
		}
	}

	if (i >= MAX_ACTIVE_REGIONS) {
        return;
	}

	/* Note: this does't add entries in order */
	early_node_map[i].nid = nid;
	early_node_map[i].start_pfn = start_pfn;
	early_node_map[i].end_pfn = end_pfn;
	nr_nodemap_entries = i + 1;
}

#define ZONELIST_ORDER_DEFAULT   0
#define ZONELIST_ORDER_NODE   1
#define ZONELIST_ORDER_ZONE   2

static int current_zonelist_order = ZONELIST_ORDER_DEFAULT;

static void zoneref_set_zone(struct zone *zone, struct zoneref *zoneref)
{
    zoneref->zone = zone;
	zoneref->zone_idx = zone_idx(zone);
}

static int build_zonelists_node(pg_data_t *pgdat, struct zonelist *zonelist, int nr_zones,
		enum zone_type zone_type)
{
    struct zone *zone;
	zone_type++;

	do {
        zone_type--;
		zone = pgdat->node_zones + zone_type;
		if (populated_zone(zone)) {
            zoneref_set_zone(zone, &zonelist->_zonerefs[nr_zones++]);
			/* check_highest_zone(zone_type); */
		}
	} while (zone_type);
	return zone_type;
}

static void build_zonelists(pg_data_t *pgdat)
{
    int node, local_node;
	enum zone_type j;
	struct zonelist *zonelist;

	local_node = pgdat->node_id;
	zonelist = &pgdat->node_zonelists[0];
	j = build_zonelists_node(pgdat, zonelist, 0, MAX_NR_ZONES - 1);

	for (node = local_node + 1; node < MAX_NUMNODES; node++) {
        if (!node_online(node))
			continue;
		j = build_zonelists_node(NODE_DATA(node), zonelist, j, MAX_NR_ZONES - 1);
	}

	for (node = 0; node < local_node; node++) {
        if (!node_online(node))
			continue;
		j = build_zonelists_node(NODE_DATA(node), zonelist, j, MAX_NR_ZONES - 1);
	}

	zonelist->_zonerefs[j].zone = NULL;
	zonelist->_zonerefs[j].zone_idx = 0;
}

static int __build_all_zonelists(void *data)
{
    int nid;

	for_each_online_node(nid) {
        pg_data_t *pgdat = NODE_DATA(nid);
		
		build_zonelists(pgdat);
	}

	return 0;
}


void build_all_zonelists(void *data)
{
	current_zonelist_order = ZONELIST_ORDER_ZONE;

    __build_all_zonelists(NULL);

	page_group_by_mobility_disabled = 1;
}

static unsigned long find_min_pfn_for_node(int nid)
{
    int i;
	unsigned long min_pfn = ULONG_MAX;

	for_each_active_range_index_in_nid(i, nid)
		min_pfn = min(min_pfn, early_node_map[i].start_pfn);

	if (min_pfn == ULONG_MAX) {
        return 0;
	}
	return min_pfn;
}

unsigned long find_min_pfn_with_active_regions(void)
{
    return find_min_pfn_for_node(MAX_NUMNODES);
}

static void find_zone_movable_pfns_for_nodes(unsigned long *movable_pfn)
{
    int i, nid;
    
}

void get_pfn_range_for_nid(unsigned int nid, unsigned long *start_pfn, unsigned long *end_pfn)
{
    int i;
	*start_pfn = -1UL;
	*end_pfn = 0;

	for_each_active_range_index_in_nid(i, nid) {
        *start_pfn = min(*start_pfn, early_node_map[i].start_pfn);
		*end_pfn = max(*end_pfn, early_node_map[i].end_pfn);
	}

	if (*start_pfn == -1UL)
		*start_pfn = 0;
}

static void adjust_zone_range_for_zone_movable(int nid, unsigned long zone_type, 
		unsigned long node_start_pfn, unsigned long node_end_pfn, unsigned long *zone_start_pfn,
		unsigned long *zone_end_pfn)
{
    if (zone_movable_pfn[nid]) {
         if (zone_type == ZONE_MOVABLE) {
             *zone_start_pfn = zone_movable_pfn[nid];
			 *zone_end_pfn = min(node_end_pfn, arch_zone_highest_possible_pfn[movable_zone]);
		 } else if (*zone_start_pfn < zone_movable_pfn[nid] && 
		            *zone_end_pfn > zone_movable_pfn[nid]) {
             *zone_end_pfn = zone_movable_pfn[nid];
		 } else if (*zone_start_pfn >= zone_movable_pfn[nid])
			 *zone_start_pfn = *zone_end_pfn;
	}
}

static unsigned long zone_spanned_pages_in_node(int nid, unsigned long zone_type, unsigned long *ignored)
{
    unsigned long node_start_pfn, node_end_pfn;
	unsigned long zone_start_pfn, zone_end_pfn;

	get_pfn_range_for_nid(nid, &node_start_pfn, &node_end_pfn);
	zone_start_pfn = arch_zone_lowest_possible_pfn[zone_type];
	zone_end_pfn = arch_zone_highest_possible_pfn[zone_type];
	adjust_zone_range_for_zone_movable(nid, zone_type, node_start_pfn, node_end_pfn, 
			&zone_start_pfn, &zone_end_pfn);

	if (zone_end_pfn < node_start_pfn || zone_start_pfn > node_end_pfn)
		return 0;

	zone_end_pfn = min(zone_end_pfn, node_end_pfn);
	zone_start_pfn = max(zone_start_pfn, node_start_pfn);

	return zone_end_pfn - zone_start_pfn;
}


unsigned long __absent_pages_in_range(int nid, unsigned long range_start_pfn, unsigned long range_end_pfn)
{
    int i = 0;
	unsigned long prev_end_pfn = 0, hole_pages = 0;
	unsigned long start_pfn = 0;

	/* By harvey, I made some changes and I suppose they render the codes below more readable */
	i = first_active_region_index_in_nid(nid);
	for (; i != -1; i = next_active_region_index_in_nid(i, nid)) {
        if (prev_end_pfn >= range_end_pfn)
			break;
		start_pfn = min(early_node_map[i].start_pfn, range_end_pfn);
		prev_end_pfn = max(prev_end_pfn, range_start_pfn);

		if (start_pfn > range_start_pfn) {
            hole_pages += start_pfn - prev_end_pfn;
		}
		prev_end_pfn = early_node_map[i].end_pfn;
	}
	if (range_end_pfn > prev_end_pfn)
		hole_pages += range_end_pfn - max(prev_end_pfn, range_start_pfn);
	return hole_pages;
}

static unsigned long zone_absent_pages_in_node(int nid, unsigned long zone_type, unsigned long *ignored)
{
    unsigned long node_start_pfn, node_end_pfn;
	unsigned long zone_start_pfn, zone_end_pfn;

	get_pfn_range_for_nid(nid, &node_start_pfn, &node_end_pfn);
	zone_start_pfn = max(arch_zone_lowest_possible_pfn[zone_type], node_start_pfn);
	zone_end_pfn = min(arch_zone_highest_possible_pfn[zone_type], node_end_pfn);
	
	adjust_zone_range_for_zone_movable(nid, zone_type, node_start_pfn, node_end_pfn,
			&zone_start_pfn, &zone_end_pfn);

	return __absent_pages_in_range(nid, zone_start_pfn, zone_end_pfn);
}

static void calculate_node_totalpages(struct pglist_data *pgdat, unsigned long *zones_size, unsigned long *zholes_size)
{
    unsigned long realtotalpages, totalpages = 0;
	enum zone_type i;

	for (i = 0; i < MAX_NR_ZONES; i++)
        totalpages += zone_spanned_pages_in_node(pgdat->node_id, i, zones_size);

	pgdat->node_spanned_pages = totalpages;

	realtotalpages = totalpages;
	for (i = 0; i < MAX_NR_ZONES; i++)
		realtotalpages -= zone_absent_pages_in_node(pgdat->node_id, i, zholes_size);

	pgdat->node_present_pages = realtotalpages;

}

static inline int pfn_to_bitidx(struct zone *zone, unsigned long pfn)
{
    pfn = pfn - zone->zone_start_pfn;
	return (pfn >> pageblock_order) * NR_PAGEBLOCK_BITS;
}

static inline unsigned long *get_pageblock_bitmap(struct zone *zone, unsigned long pfn)
{
    return zone->pageblock_flags;
}

void set_pageblock_flags_group(struct page *page, unsigned long flags, int start_bitidx, int end_bitidx)
{
    struct zone *zone;
	unsigned long *bitmap;
	unsigned long pfn, bitidx;
	unsigned long value = 1;

	zone = page_zone(page);
	pfn = page_to_pfn(page);
	bitmap = get_pageblock_bitmap(zone, pfn);
	bitidx = pfn_to_bitidx(zone, pfn);

	for (; start_bitidx <= end_bitidx; start_bitidx++, value <<= 1)
	{
        if (flags & value)
			__set_bit(bitidx + start_bitidx, bitmap);
		else
			__clear_bit(bitidx + start_bitidx, bitmap);
	}
}

static void set_pageblock_migratetype(struct page *page, int migratetype)
{
    if (page_group_by_mobility_disabled)
		migratetype = MIGRATE_UNMOVABLE;
	set_pageblock_flags_group(page, (unsigned long)migratetype, PB_migrate, PB_migrate_end);
}

#define memmap_init(size, nid, zone, start_pfn)    \
	memmap_init_zone((size), (nid), (zone), (start_pfn), MEMMAP_EARLY)

void memmap_init_zone(unsigned long size, int nid, unsigned long zone, unsigned long start_pfn, 
		enum memmap_context context)
{
    struct page *page;
	unsigned long end_pfn = start_pfn + size;
	unsigned long pfn;
	struct zone *z;

	if (highest_memmap_pfn < end_pfn - 1)
		highest_memmap_pfn = end_pfn - 1;

	z = &NODE_DATA(nid)->node_zones[zone];
	for (pfn = start_pfn; pfn < end_pfn; pfn++) {
        if (context == MEMMAP_EARLY) {
            if (!early_pfn_valid(pfn))
				continue;
			if (!early_pfn_in_nid(pfn, nid))
				continue;
		}

		page = pfn_to_page(pfn);
		set_page_links(page, zone, nid, pfn);
        init_page_count(page);
        reset_page_mapcount(page);

		if ((z->zone_start_pfn <= pfn) && (pfn < z->zone_start_pfn + z->spanned_pages) 
				&& !(pfn & (pageblock_nr_pages - 1)))
			set_pageblock_migratetype(page, MIGRATE_MOVABLE);

		INIT_LIST_HEAD(&page->lru);
    }
}

static void zone_pcp_init(struct zone *zone)
{
    zone->pageset = &boot_pageset;
}

/* unused */
#define set_pageblock_order(x) do {} while (0)
/* unused */
static int pageblock_default_order(void)
{
	return MAX_ORDER - 1;
}

static void zone_init_free_lists(struct zone *zone)
{
    int order, t;
	for_each_migratetype_order(order, t) {
        INIT_LIST_HEAD(&zone->free_area[order].free_list[t]);
		zone->free_area[order].nr_free = 0;
	}
}

static int cmp_node_active_region(const void *a, const void *b)
{
    struct node_active_region *arange = (struct node_active_region *)a;
    struct node_active_region *brange = (struct node_active_region *)b;

	if (arange->start_pfn > brange->start_pfn)
		return 1;
	if (arange->start_pfn < brange->start_pfn)
		return -1;
	return 0;
}

void sort_node_map(void)
{
    sort(early_node_map, (size_t)nr_nodemap_entries, sizeof(struct node_active_region), cmp_node_active_region, NULL);
}

#define PAGES_PER_WAITQUEUE 256
/* Don't support hotplug */
static inline unsigned long wait_table_hash_nr_entries(unsigned long pages)
{
    unsigned long size = 1;
	pages /= PAGES_PER_WAITQUEUE;

	while (size < pages)
		size <<= 1;

	size = min(size, 4096UL);
	
	return max(size, 4UL);
}

static inline unsigned long wait_table_bits(unsigned long size)
{
    return ffz(~size);
}

int zone_wait_table_init(struct zone *zone, unsigned long zone_size_pages)
{
    int i;
	struct pglist_data *pgdat = zone->zone_pgdat;
	size_t alloc_size;

	zone->wait_table_hash_nr_entries = wait_table_hash_nr_entries(zone_size_pages);
	zone->wait_table_bits = wait_table_bits(zone->wait_table_hash_nr_entries);
	alloc_size = zone->wait_table_hash_nr_entries * sizeof(wait_queue_head_t);

	if (!slab_is_available()) {
        zone->wait_table = (wait_queue_head_t *)alloc_bootmem_node_nopanic(pgdat, alloc_size);
	} else {
		/* I don't know how it works */
        zone->wait_table = vmalloc(alloc_size); 
	}

	if (!zone->wait_table)
		return -ENOMEM;

	for (i = 0; i < zone->wait_table_hash_nr_entries; ++i)
		init_waitqueue_head(zone->wait_table + i);

	return 0;
}

int init_currently_empty_zone(struct zone *zone, unsigned long zone_start_pfn, unsigned long size, 
		enum memmap_context context)
{
    struct pglist_data *pgdat = zone->zone_pgdat;
	int ret;
	ret = zone_wait_table_init(zone, size);
	if (ret)
		return ret;
	pgdat->nr_zones = zone_idx(zone) + 1;
	zone->zone_start_pfn = zone_start_pfn;
	zone_init_free_lists(zone);

	return 0;
}

/* Don't support SPARSEMEM, that is, CONFIG_SPARSEMEM is defined */
static unsigned long usemap_size(unsigned long zonesize)
{
    unsigned long usemapsize;
	usemapsize = roundup(zonesize, pageblock_nr_pages);
	usemapsize = usemapsize >> pageblock_order;
	usemapsize *= NR_PAGEBLOCK_BITS;
	usemapsize = roundup(usemapsize, 8 * sizeof(unsigned long));
	return usemapsize / 8;
}

static void setup_usemap(struct pglist_data *pgdat, struct zone *zone, unsigned long zonesize)
{
    unsigned long usemapsize = usemap_size(zonesize);
	zone->pageblock_flags = NULL;
	if (usemapsize)
		zone->pageblock_flags = alloc_bootmem_node_nopanic(pgdat, usemapsize);
}

static void free_area_init_core(struct pglist_data *pgdat, unsigned long *zones_size, unsigned long *zholes_size)
{
    enum zone_type j;
	int nid = pgdat->node_id;
	unsigned long zone_start_pfn = pgdat->node_start_pfn;
	int ret;

	/* Don't support memory hotplug */
	/* pgdat_resize_init(pgdat); */
	pgdat->nr_zones = 0;
	init_waitqueue_head(&pgdat->kswapd_wait);
	pgdat->kswapd_max_order = 0;
	/* Don't support Page Cgroup */
	/* pgdat_page_cgroup_init(pgdat); */
	for (j = 0; j < MAX_NR_ZONES; j++) {
        struct zone *zone = pgdat->node_zones + j;
		unsigned long size, realsize, memmap_pages;
		enum lru_list l;

		size = zone_spanned_pages_in_node(nid, j, zones_size);
		realsize = size - zone_absent_pages_in_node(nid, j, zholes_size);

		memmap_pages = PAGE_ALIGN(size * sizeof(struct page)) >> PAGE_SHIFT;
		if (realsize >= memmap_pages)
			realsize -= memmap_pages;
		
		if (j == 0 && realsize > dma_reserve)
			realsize -= dma_reserve;

		if (!is_highmem_idx(j))
			nr_kernel_pages += realsize;
		nr_all_pages += realsize;

		zone->spanned_pages = size;
		zone->present_pages = realsize;
		zone->name = zone_names[j];
		spin_lock_init(&zone->lock);
		spin_lock_init(&zone->lru_lock);
		/* Don't support memory-hotplug */
		zone_seqlock_init(zone);
		zone->zone_pgdat = pgdat;
		
		zone_pcp_init(zone);
		for_each_lru(l)
			INIT_LIST_HEAD(&zone->lru[l].list);
		zone->reclaim_stat.recent_rotated[0] = 0;
        zone->reclaim_stat.recent_rotated[1] = 0;
        zone->reclaim_stat.recent_scanned[0] = 0;
        zone->reclaim_stat.recent_scanned[1] = 0;
		memset(zone->vm_stat, 0, sizeof(zone->vm_stat));
		zone->flags = 0;
		if (!size)
			continue;

		set_pageblock_order(pageblock_default_order());
		setup_usemap(pgdat, zone, size);

		ret = init_currently_empty_zone(zone, zone_start_pfn, size, MEMMAP_EARLY);
		memmap_init(size, nid, j, zone_start_pfn);
		zone_start_pfn += size;
	}
}

static void alloc_node_mem_map(struct pglist_data *pgdat)
{
    if (!pgdat->node_spanned_pages)
		return;

	if (!pgdat->node_mem_map) {
        unsigned long size, start, end;
		struct page *map;

		start = pgdat->node_start_pfn & ~(MAX_ORDER_NR_PAGES - 1);
		end = pgdat->node_start_pfn + pgdat->node_spanned_pages;
		end = ALIGN(end, MAX_ORDER_NR_PAGES);
		size = (end - start) * sizeof(struct page);
		map = alloc_remap(pgdat->node_id, size);
		if (!map)
			map = alloc_bootmem_node_nopanic(pgdat, size);
		pgdat->node_mem_map = map + (pgdat->node_start_pfn - start);
	}

	if (pgdat == NODE_DATA(0)) {
	    mem_map = NODE_DATA(0)->node_mem_map;
		if (page_to_pfn(mem_map) != pgdat->node_start_pfn)
			mem_map -= (pgdat->node_start_pfn - ARCH_PFN_OFFSET);
	}
}

void free_area_init_node(int nid, unsigned long *zones_size, unsigned long node_start_pfn, unsigned long  *zholes_size)
{
    pg_data_t *pgdat = NODE_DATA(nid);

	pgdat->node_id = nid;
	pgdat->node_start_pfn = node_start_pfn;
	calculate_node_totalpages(pgdat, zones_size, zholes_size);

	alloc_node_mem_map(pgdat);
	free_area_init_core(pgdat, zones_size, zholes_size);
}

void free_area_init_nodes(unsigned long *max_zone_pfn)
{
    unsigned long nid;
    int i;

	sort_node_map();
    memset(arch_zone_lowest_possible_pfn, 0, sizeof(arch_zone_lowest_possible_pfn));
	memset(arch_zone_highest_possible_pfn, 0, sizeof(arch_zone_highest_possible_pfn));
	arch_zone_lowest_possible_pfn[0] = find_min_pfn_with_active_regions();
	arch_zone_highest_possible_pfn[0] = max_zone_pfn[0];

	for(i = 1; i < MAX_NR_ZONES; i++) {
        if (i == ZONE_MOVABLE)
			continue;
		arch_zone_lowest_possible_pfn[i] = arch_zone_highest_possible_pfn[i - 1];
		arch_zone_highest_possible_pfn[i] = max(max_zone_pfn[i], arch_zone_lowest_possible_pfn[i]);
	}
	arch_zone_lowest_possible_pfn[ZONE_MOVABLE] = 0;
	arch_zone_highest_possible_pfn[ZONE_MOVABLE] = 0;

	memset(zone_movable_pfn, 0, sizeof(zone_movable_pfn));
	find_zone_movable_pfns_for_nodes(zone_movable_pfn);
	for_each_online_node(nid) {
        pg_data_t *pgdat = NODE_DATA(nid);
		free_area_init_node(nid, NULL, find_min_pfn_for_node(nid), NULL);
		if (pgdat->node_present_pages)
			node_set_state(nid, N_HIGH_MEMORY);
	}
}


static inline bool mem_cgroup_bad_page_check(struct page *page)
{
    return  false;
}

static void bad_page(struct page *page)
{

}

static inline int free_pages_check(struct page *page)
{
    if (page_mapcount(page) | (page->mapping != NULL) | (page->flags & PAGE_FLAGS_CHECK_AT_FREE) | 
			(mem_cgroup_bad_page_check(page))) {
        bad_page(page);
		return 1;
	}
	if (page->flags & PAGE_FLAGS_CHECK_AT_PREP)
		page->flags &= ~PAGE_FLAGS_CHECK_AT_PREP;
	return 0;
}

static bool free_pages_prepare(struct page *page, unsigned int order)
{
    int i;
	int bad = 0;

    if (PageAnon(page))
		page->mapping = NULL;
	for (i = 0; i < (1 << order); i++)
		bad += free_pages_check(page + i);
	if (bad)
		return false;

	return true;
}


static int destroy_compound_page(struct page *page, unsigned long order)
{
    int i;
	int nr_pages = 1 << order;
	int bad = 0;

	if ((compound_order(page) != order) || (!PageHead(page))) {
        bad_page(page);
		bad++;
	}

    __ClearPageHead(page);

	for (i = 1; i < nr_pages; i++) {
        struct page *p = page + i;
		if (!PageTail(p) || (p->first_page != page)) {
            bad_page(page);
			bad++;
		}
		__ClearPageTail(page);
	}
	return bad;
}

static inline void rmv_page_order(struct page *page)
{
    __ClearPageBuddy(page);
	set_page_private(page, 0);
}

static inline void set_page_order(struct page *page, int order)
{
    set_page_private(page, order);
	__SetPageBuddy(page);
}

static inline unsigned long __find_buddy_index(unsigned long page_idx, unsigned long order)
{
   return page_idx ^ (1 << order);
}

static inline int page_is_buddy(struct page *page, struct page *buddy, int order)
{
    if (!pfn_valid_within(page_to_pfn(buddy)))
		return 0;
	if (page_zone_id(page) != page_zone_id(buddy))
		return 0;
	if (PageBuddy(buddy) && page_order(buddy) == order) {
        return 1;
	}
	return 0;
}

static inline void __free_one_page(struct page *page, struct zone *zone, unsigned int order, int migratetype)
{
    unsigned long page_idx;
	unsigned long combined_idx;
	unsigned long uninitialized_var(buddy_idx);
	struct page *buddy;

	if (PageCompound(page))
		if (destroy_compound_page(page, order))
			return;

	page_idx = page_to_pfn(page) & ((1 << MAX_ORDER) - 1);
    
	while (order < MAX_ORDER - 1) {
        buddy_idx = __find_buddy_index(page_idx, order);
		buddy = page + (buddy_idx - page_idx);
		if (!page_is_buddy(page, buddy, order))
			break;

		list_del(&buddy->lru);
		zone->free_area[order].nr_free--;
		rmv_page_order(buddy);
		combined_idx = buddy_idx & page_idx;
		page = page + (combined_idx - page_idx);
		page_idx = combined_idx;
		order++;
	}
	set_page_order(page, order);

	if ((order < MAX_ORDER - 2) && pfn_valid_within(page_to_pfn(buddy))) {
        struct page *higher_page, *higher_buddy;
		combined_idx = buddy_idx & page_idx;
		higher_page = page + (combined_idx - page_idx);
		buddy_idx = __find_buddy_index(combined_idx, order + 1);
		higher_buddy = page + (buddy_idx - combined_idx);
		if (page_is_buddy(higher_page, higher_buddy, order + 1)) {
            list_add_tail(&page->lru, &zone->free_area[order].free_list[migratetype]);
			goto out;
		}
	}
	list_add(&page->lru, &zone->free_area[order].free_list[migratetype]);
out:
	zone->free_area[order].nr_free++;
}

static void free_pcppages_bulk(struct zone *zone, int count, struct per_cpu_pages *pcp)
{
    int migratetype = 0;
	int batch_free = 0;
	int to_free = count;

	spin_lock(&zone->lock);
	zone->all_unreclaimable = 0;
	zone->pages_scanned = 0;

	while (to_free) {
        struct page *page;
		struct list_head *list;

		do {
            batch_free++;
			if (++migratetype == MIGRATE_PCPTYPES)
				migratetype = 0;
			list = &pcp->lists[migratetype];
		} while (list_empty(list));

		if (batch_free == MIGRATE_PCPTYPES)
			batch_free = to_free;

		do {
            page = list_entry(list->prev, struct page, lru);
			list_del(&page->lru);
			__free_one_page(page, zone, 0, page_private(page));
            /* trace_mm_page_pcpu_drain(page, 0, page_private(page)); */
		} while (--to_free && --batch_free && !list_empty(list));
	}
	__mod_zone_page_state(zone, NR_FREE_PAGES, count);
	spin_unlock(&zone->lock);
}

static void free_one_page(struct zone *zone, struct page *page, int order, int migratetype)
{
    spin_lock(&zone->lock);
	zone->all_unreclaimable = 0;
	zone->pages_scanned = 0;

	__free_one_page(page, zone, order, migratetype);
	__mod_zone_page_state(zone, NR_FREE_PAGES, 1 << order);
	spin_unlock(&zone->lock);
}

void free_hot_cold_page(struct page *page, int cold)
{
    struct zone *zone = page_zone(page);
	struct per_cpu_pages *pcp;
	unsigned long flags;
	int migratetype;
	/* int wasMlocked = __TestClearPageMlocked(page); */

	if (!free_pages_prepare(page, 0))
		return;

    migratetype = get_pageblock_migratetype(page);
	set_page_private(page, migratetype);
	local_irq_save(flags);
	/*
	if (wasMlocked)
		free_page_mlock(page);
    __count_vm_event(PGFREE);
	*/

	if (migratetype >= MIGRATE_PCPTYPES) {
        if (migratetype == MIGRATE_ISOLATE) {
            free_one_page(zone, page, 0, migratetype);
			goto out;
		}
		migratetype = MIGRATE_MOVABLE;
	}
	pcp = &this_cpu_ptr(zone->pageset)->pcp;
	if (cold)
		list_add_tail(&page->lru, &pcp->lists[migratetype]);
	else
		list_add(&page->lru, &pcp->lists[migratetype]);
	pcp->count++;
	if (pcp->count >= pcp->high) {
        free_pcppages_bulk(zone, pcp->batch, pcp);
		pcp->count -= pcp->batch;
	}
out:
	local_irq_restore(flags);
}


static void __free_pages_ok(struct page *page, unsigned int order)
{
    unsigned long flags;
	/* int wasMlocked = __TestClearPageMlocked(page); */
	
	if (!free_pages_prepare(page, order))
		return;

	local_irq_save(flags);
	/*
	 * if (wasMlocked)
	 *	   free_page_mlock(page);
	 * __count_vm_events(PGFREE, 1 << order);
	 */
	free_one_page(page_zone(page), page, order, get_pageblock_migratetype(page));
	local_irq_restore(flags);
}

void __free_pages(struct page *page, unsigned int order)
{
    if (put_page_testzero(page)) {
        if (order == 0)
			free_hot_cold_page(page, 0);
		else
			__free_pages_ok(page, order);
	}
}

void free_pages(unsigned long addr, unsigned int order)
{
    if (addr != 0) {
        /* VM_BUG_ON(!virt_addr_valid((void *)addr)); */
		__free_pages(virt_to_page((void *)addr), order);
	}
}

static bool __zone_watermark_ok(struct zone *z, int order, unsigned long mark,
		                        int classzone_idx, int alloc_flags, long free_pages)
{
    long min = mark;
	int o;

	free_pages -= (1 << order) + 1;
	if (alloc_flags & ALLOC_HIGH)
		min -= min / 2;
	if (alloc_flags & ALLOC_HARDER)
		min -= min / 4;

	if (free_pages <= min + z->lowmem_reserve[classzone_idx])
		return false;
	for (o = 0; o < order; o++) {
        free_pages -= z->free_area[o].nr_free << o;
		min >>= 1;
		if (free_pages <= min)
			return false;
	}
	return true;
}

bool zone_watermark_ok(struct zone *z, int order, unsigned long mark, 
		int classzone_idx, int alloc_flags)
{
    return __zone_watermark_ok(z, order, mark, classzone_idx, alloc_flags, 
			zone_page_state(z, NR_FREE_PAGES));
}

static inline void expand(struct zone *zone, struct page *page, int low, int high, 
		                  struct free_area *area, int migratetype)
{
    unsigned long size = 1 << high;

	while (high > low) {
        area--;
		high--;
		size >>= 1;
		list_add(&page[size].lru, &area->free_list[migratetype]);
		area->nr_free++;
		set_page_order(&page[size], high);
	}
}

static int move_freepages(struct zone *zone, struct page *start_page, struct page *end_page, 
		int migratetype)
{
    struct page *page;
	unsigned long order;
	int pages_moved = 0;

	for (page = start_page; page <= end_page;) {
        if (!pfn_valid_within(page_to_pfn(page))) {
            page++;
			continue;
		}

		if (!PageBuddy(page)) {
            page++;
			continue;
		}

		order = page_order(page);
		list_move(&page->lru, &zone->free_area[order].free_list[migratetype]);
		page += 1 << order;
		pages_moved += 1 << order;
	}

	return pages_moved;
}

static int move_freepages_block(struct zone *zone, struct page *page, int migratetype)
{
    unsigned long start_pfn, end_pfn;
	struct page *start_page, *end_page;

	start_pfn = page_to_pfn(page);
	start_pfn = start_pfn & ~(pageblock_nr_pages - 1);
	start_page = pfn_to_page(start_pfn);
	end_page = start_page + pageblock_nr_pages - 1;
	end_pfn = start_pfn + pageblock_nr_pages - 1;

	if (start_pfn < zone->zone_start_pfn)
		start_page = page;
	if (end_pfn >= zone->zone_start_pfn + zone->spanned_pages)
		return 0;

	return move_freepages(zone, start_page, end_page, migratetype);
}


static void change_pageblock_range(struct page *pageblock_page, int start_order, int migratetype)
{
    int nr_pageblocks = 1 << (start_order - pageblock_order);
	while (nr_pageblocks--) {
        set_pageblock_migratetype(pageblock_page, migratetype);
		pageblock_page += pageblock_nr_pages;
	}
}

static inline int check_new_page(struct page * page)
{
    if (page_mapcount(page) | (page->mapping != NULL) | (atomic_read(&page->_count) != 0) |
				(page->flags & PAGE_FLAGS_CHECK_AT_PREP) | (mem_cgroup_bad_page_check(page))) {
        bad_page(page);
		return 1;
	}
	return 0;
}

static inline void prep_zero_page(struct page *page, int order, gfp_t gfp_flags)
{
    int i;

	/* VM_BUG_ON((gfp_flags & __GFP_HIGHMEM) && in_interrupt());
	for (i = 0; i < (1 << order); i++)
		clear_highpage(page + i);
	*/
	/*By harvey, choose an simpler way*/
	for (i = 0; i < (1 << order); i++)
		clear_page(page + i);
}

static void free_compound_page(struct page *page)
{
    __free_pages_ok(page, compound_order(page));
}

void prep_compound_page(struct page *page, unsigned long order)
{
    int i;
	int nr_pages = 1 << order;

	set_compound_page_dtor(page, free_compound_page);
	set_compound_order(page, order);
	__SetPageHead(page);
	for (i = 1; i < nr_pages; i++) {
        struct page *p = page + i;
		__SetPageTail(p);
		p->first_page = page;
	}
}

static int prep_new_page(struct page *page, int order, gfp_t gfp_flags)
{
    int i;

	for (i = 0; i < (1 << order); i++) {
        struct page *p = page + i;
		if (check_new_page(p))
			return 1;
	}

	set_page_private(page, 0);
	set_page_refcounted(page);
	
	arch_alloc_page(page, order);
	kernel_map_pages(page, 1 << order, 1);

	if (gfp_flags & __GFP_ZERO)
		prep_zero_page(page, order, gfp_flags);
	if (order && (gfp_flags & __GFP_COMP))
		prep_compound_page(page, order);
	
	 return 0;
}


static inline struct page *__rmqueue_smallest(struct zone *zone, unsigned int order, 
		                                      int migratetype)
{
    unsigned int current_order;
	struct free_area *area;
	struct page *page;

	for (current_order = order; current_order < MAX_ORDER; ++current_order) {
        area = &(zone->free_area[current_order]);
		if (list_empty(&area->free_list[migratetype]))
			continue;
		page = list_entry(area->free_list[migratetype].next, struct page, lru);
		list_del(&page->lru);
		rmv_page_order(page);
		area->nr_free--;
		expand(zone, page, order, current_order, area, migratetype);
		return page;
	}
	return NULL;
}

static inline struct page *__rmqueue_fallback(struct zone *zone, int order, int start_migratetype)
{
    struct free_area *area;
	int current_order;
	struct page *page;

	int migratetype, i;
	for (current_order = MAX_ORDER - 1; current_order >= order; --current_order) {
        for (i = 0; i < MIGRATE_TYPES; i++) {
            migratetype = fallbacks[start_migratetype][i];

			if (migratetype == MIGRATE_RESERVE)
				continue;

			area = &(zone->free_area[current_order]);
			if (list_empty(&area->free_list[migratetype]))
				continue;

			page = list_entry(area->free_list[migratetype].next, struct page, lru);
			area->nr_free--;

			if ((current_order >= (pageblock_order >> 1)) || 
			     start_migratetype == MIGRATE_RECLAIMABLE || 
				 page_group_by_mobility_disabled) {
                unsigned long pages;
				pages = move_freepages_block(zone, page, start_migratetype);

				if (pages >= (1 << (pageblock_order - 1)) || 
					page_group_by_mobility_disabled)
					set_pageblock_migratetype(page, start_migratetype);

				migratetype = start_migratetype;
			}

			list_del(&page->lru);
			rmv_page_order(page);

			if (current_order >= pageblock_order)
				change_pageblock_range(page, current_order, start_migratetype);

			expand(zone, page, order, current_order, area, migratetype);

			/* trace_mm_page_alloc_extfraq(page, order, current_order, start_migratetype, 
					                    migratetype); */
			return page;
		}
	}
	return NULL;
}

static struct page *__rmqueue(struct zone *zone, unsigned int order, int migratetype)
{
    struct page *page;

retry_reserve:
	page = __rmqueue_smallest(zone, order, migratetype);
	if ((!page) && migratetype != MIGRATE_RESERVE) {
        page = __rmqueue_fallback(zone, order, migratetype);
		if (!page) {
            migratetype = MIGRATE_RESERVE;
			goto retry_reserve;
		}
	}
	/* trace_mm_page_alloc_zone_locked(page, order, migratetype); */
	return page;
}

static int rmqueue_bulk(struct zone *zone, unsigned int order, unsigned long count, 
		                struct list_head *list, int migratetype, int cold)
{
    int i;
	
	spin_lock(&zone->lock);
	for (i = 0; i < count; ++i) {
		struct page *page = __rmqueue(zone, order, migratetype);
		if (page == NULL)
			break;
		if (cold == 0)
			list_add(&page->lru, list);
		else
			list_add_tail(&page->lru, list);
		set_page_private(page, migratetype);
		list = &page->lru;
	}
	__mod_zone_page_state(zone, NR_FREE_PAGES, -(i << order));
	spin_unlock(&zone->lock);
	return i;
}

struct page *buffered_rmqueue(struct zone *preferred_zone, struct zone *zone, int order, 
		                      gfp_t gfp_flags, int migratetype)
{
    unsigned long flags;
	struct page *page;
	int cold = !!(gfp_flags & __GFP_COLD);

again:
	if (order == 0) {
        struct per_cpu_pages *pcp;
		struct list_head *list;
		local_irq_save(flags);
		pcp = &this_cpu_ptr(zone->pageset)->pcp;
		list = &pcp->lists[migratetype];
		if (list_empty(list)) {
            pcp->count += rmqueue_bulk(zone, 0, pcp->batch, list, migratetype, cold);
			if (list_empty(list))
				goto failed;
		}

		if (cold)
			page = list_entry(list->prev, struct page, lru);
		else
			page = list_entry(list->next, struct page, lru);

		list_del(&page->lru);
		pcp->count--;
	} else {
		/*
        if (gfp_flags & __GFP_NOFAIL) {
            WARN_ON_ONCE(order > 1);
		}
		*/
		spin_lock_irqsave(&zone->lock, flags);
		page = __rmqueue(zone, order, migratetype);
		spin_unlock(&zone->lock);
		if (!page)
			goto failed;
		__mod_zone_page_state(zone, NR_FREE_PAGES, -(1 << order));
	}
	/* __count_zone_vm_events(PGALLOC, zone, 1 << order); */
	zone_statistics(preferred_zone, zone, gfp_flags);
	local_irq_restore(flags);

    /* VM_BUG_ON(bad_range(zone, page)); */
	if (prep_new_page(page, order, gfp_flags))
		goto again;
	return page;

failed:
	local_irq_restore(flags);
	return NULL;
}

static nodemask_t *zlc_setup(struct zonelist *zonelist, int alloc_flags)
{
    return NULL;
}

static int zlc_zone_worth_trying(struct zonelist *zonelist, struct zoneref *z, nodemask_t *allowednodes)
{
    return 1;
}

static void zlc_mark_zone_full(struct zonelist *zonelist, struct zoneref *z) {}

static struct page *get_page_from_freelist(gfp_t gfp_mask, nodemask_t *nodemask, 
		                                unsigned int order, 
										struct zonelist *zonelist, 
										int high_zoneidx, int alloc_flags, 
										struct zone *preferred_zone, 
		                                int migratetype)
{
    struct zoneref *z;
	struct page *page = NULL;
	int classzone_idx;
	struct zone *zone;
	nodemask_t *allowednodes = NULL;
	int zlc_active = 0;
	int did_zlc_setup = 0;

	classzone_idx = zone_idx(preferred_zone);
zonelist_scan:
	for_each_zone_zonelist_nodemask(zone, z, zonelist, high_zoneidx, 
			                        nodemask) {
		if (NUMA_BUILD && zlc_active && !zlc_zone_worth_trying(zonelist, z,
					allowednodes))
			continue;
		if ((alloc_flags & ALLOC_CPUSET) && !cpuset_zone_allowed_softwall(
					zone, gfp_mask))
			continue;
		if (!(alloc_flags & ALLOC_NO_WATERMARKS)) {
            unsigned long mark;
			int ret;
			
			mark = zone->watermark[alloc_flags & ALLOC_WMARK_MASK];
			if (zone_watermark_ok(zone, order, mark, classzone_idx, 
						alloc_flags))
				goto try_this_zone;
			
			if (NUMA_BUILD && !did_zlc_setup && nr_online_nodes > 1) {
                allowednodes = zlc_setup(zonelist, alloc_flags);
				zlc_active = 1;
				did_zlc_setup = 1;
			}

			if (zone_reclaim_mode == 0)
				goto this_zone_full;

			if (NUMA_BUILD && zlc_active && !zlc_zone_worth_trying(zonelist, z, 
						                                           allowednodes))
				continue;

			ret = zone_reclaim(zone, gfp_mask, order);
			switch (ret) {
                case ZONE_RECLAIM_NOSCAN:
					continue;
				case ZONE_RECLAIM_FULL:
					continue;
				default:
					if (!zone_watermark_ok(zone, order, mark, classzone_idx, 
								           alloc_flags))
						goto this_zone_full;
			}
		}
try_this_zone:
		page = buffered_rmqueue(preferred_zone, zone, order, gfp_mask, migratetype);
		if (page)
			break;
this_zone_full:
		if (NUMA_BUILD)
			zlc_mark_zone_full(zonelist, z);
	}

	if (NUMA_BUILD && page == NULL && zlc_active) {
        zlc_active = 0;
		goto zonelist_scan;
	}
	return page;
}

/*
static inline struct page *__alloc_pages_slowpath(gfp_t gfp_mask, unsigned int order, 
		                                          struct zonelist *zonelist, 
												  enum zone_type high_zoneidx, 
												  nodemask_t *nodemask, 
												  struct zone *preferred_zone,
												  int migratetype)
{
    const gfp_t wait = gfp_mask & __GFP_WAIT;
	struct page *page = NULL;
	int alloc_flags;
	unsigned long pages_reclaimed = 0;
	unsigned long did_some_progress;
	bool svnc_migration = false;

	if (order >= MAX_ORDER) {
        return NULL;
	}

	if (NUMA_BUILD && (gfp_mask & GFP_THISNODE) == GFP_THISNODE)
		goto nopage;

restart:
	if (!(gfp_mask & __GFP_NO_KSWAPD))
		wake_all_kswapd(order, zonelist, high_zoneidx, zone_idx(preferred_zone));

    alloc_flags = gfp_to_alloc_flags(gfp_mask);

	if (!(alloc_flags & ALLOC_CPUSET) && !nodemask)
		first_zones_zonelist(zonelist, high_zoneidx, NULL, &preferred_zone);

rebalance:
	page = get_page_from_freelist(gfp_mask, nodemask, order, zonelist, high_zoneidx, 
			                      alloc_flags & ~ALLOC_NO_WATERMRKS, preferred_zone, 
								  migratetype);
	if (page)
		goto got_pg;

	if (alloc_flags & ALLOC_NO_WATERMARKS) {
        page = __alloc_pages_high_priority(gfp_mask, order, zonelist, high_zoneidx, 
				                           nodemask, preferred_zone, migratetype);
		if (page)
			goto got_pg;
    }

    if (!wait)
		goto nopage;

	if (current->flags & PF_MEMALLOC)
		goto nopage;

	if (test_thread_flag(TIF_MEMDIE) && !(gfp_mask & __GFP_NOFAIL))
		goto nopage;

	page = __alloc_pages_direct_compact(gfp_mask, order, zonelist, high_zoneidx, 
			                            nodemask, alloc_flags, preferred_zone, 
										migratetype, &did_some_progress, 
										svnc_migration);

	if (page)
		goto got_pg;
	svnc_migration = true;

	page = __alloc_pages_direct_reclaim(gfp_mask, order, zonelist, high_zoneidx, 
			                            nodemask, alloc_flags, preferred_zone, 
										migratetype, &did_some_progress);
	if (page)
		goto got_pg;

	if (!did_some_progress) {
        if ((gfp_mask & __GFP_FS) && !(gfp_mask & __GFP_NORETRY)) {
            if (oom_killer_disabled)
			    goto nopage;

			page = __alloc_pages_may_oom(gfp_mask, order, zonelist, high_zoneidx, 
					                     nodemask, preferred_zone, migratetype);
			if (page)
				goto got_pg;

			if (!(gfp_mask & __GFP_NOFAIL)) {
                if (order > PAGE_ALLOC_COSTLY_ORDER)
					goto nopage;

				if (high_zoneidx < ZONE_NORMAL)
					goto nopage;
			}
			goto restart;
		}
	}

	pages_reclaimed += did_some_progress;
	if (should_alloc_retry(gfp_mask, order, pages_reclaimed)) {
        wait_iff_congested(preferred_zone, BLK_RW_ASYNC, HZ / 50);
		goto rebalance;
	} else {
        page = __alloc_pages_direct_compact(gfp_mask, order, zonelist, high_zoneidx, 
				                            nodemask, alloc_flags, preferred_zone, 
											migratetype, &did_some_progress, 
											sync_migration);
		if (page)
			goto got_pg;
	}

nopage:
	warn_alloc_failed(gfp_order, order, NULL);
	return page;
got_pg:
	if (kmemcheck_enabled)
		kmemcheck_pagealloc_alloc(page, order, gfp_mask);
	return page;
}
*/

struct page *__alloc_pages_nodemask(gfp_t gfp_mask, unsigned int order, 
		                            struct zonelist *zonelist, nodemask_t *nodemask)
{
    enum zone_type high_zoneidx = gfp_zone(gfp_mask);
	struct zone *preferred_zone;
	struct page *page;
	int migratetype = allocflags_to_migratetype(gfp_mask);

	gfp_mask &= gfp_allowed_mask;
    
	/* lockdep_trace_alloc(gfp_mask); */
	might_sleep_if(gfp_mask & __GFP_WAIT);
	 
	/* CONFIG_FAIL_PAGE_ALLOC is not defined */
	/*
	 * if (should_fail_alloc_page(gfp_mask, order))
	 *     return NULL;
	 */

	if (!zonelist->_zonerefs->zone)
		return NULL;

    /* CONFIG_CPUSETS is not defined */
	/* get_mems_allowed(); */
	first_zones_zonelist(zonelist, high_zoneidx, 
			             nodemask ? : &cpuset_current_mems_allowed, &preferred_zone);

	if (!preferred_zone) {
        /* put_mems_allowed(); */
		return NULL;
	}

	page = get_page_from_freelist(gfp_mask | __GFP_HARDWALL, nodemask, order, 
			                      zonelist, high_zoneidx, 
								  ALLOC_WMARK_LOW | ALLOC_CPUSET, preferred_zone, 
								  migratetype);

	/*
	if (!page)
		page = __alloc_pages_slowpath(gfp_mask, order, zonelist, high_zoneidx, 
				                      nodemask, preferred_zone,	migratetype);
	*/

    /* CONFIG_CPUSETS is not defined */
	/* put_mems_allowed(); */
	
	return NULL;
}

unsigned long __get_free_pages(gfp_t gfp_mask, unsigned int order)
{
    struct page *page;

	page = alloc_pages(gfp_mask, order);
	if (!page)
		return 0;
	return (unsigned long)page_address(page);
}

unsigned long get_pageblock_flags_group(struct page *page, int start_bitidx, int end_bitidx)
{
    struct zone *zone;
	unsigned long *bitmap;
	unsigned long pfn, bitidx;
	unsigned long flags = 0;
	unsigned long value = 1;

	zone = page_zone(page);
	pfn = page_to_pfn(page);
	bitmap = get_pageblock_bitmap(zone, pfn);
	bitidx = pfn_to_bitidx(zone, pfn);

	for (; start_bitidx <= end_bitidx; start_bitidx++, value <<=1 )
		if (test_bit(bitidx + start_bitidx, bitmap))
			flags |= value;

	return flags;
}
