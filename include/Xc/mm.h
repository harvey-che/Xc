#ifndef _XC_MM_H
#define _XC_MM_H

#include <Xc/types.h>
#include <asm/page.h>
#include <asm/pgtable_types.h>
#include <Xc/kernel.h>
#include <Xc/mmzone.h>
#include <Xc/mm_types.h>
#include <Xc/atomic.h>
#include <Xc/page-flags.h>
#include <Xc/pfn.h>
#include <Xc/range.h>

/* pageblock_flags.h */

#define pageblock_order HUGETLB_PAGE_ORDER
#define pageblock_nr_pages (1UL << pageblock_order)

/* end -- pageblock_flags.h */

void early_mem_init();
void setup_paging(void);
void free_area_init_nodes(unsigned long *max_zone_pfn);
void zone_sizes_init(void);

void get_pfn_range_for_nid(unsigned int nid, unsigned long *start_pfn, unsigned long *end_pfn);

u64 find_memory_core_early(int nid, u64 size, u64 align, u64 goal, u64 limit);
extern void add_active_range(unsigned int nid, unsigned long start_pfn, unsigned long end_pfn);
extern void mem_init(void);

int add_from_early_node_map(struct range *range, int az, int nr_range, int nid);
typedef int (*work_fn_t)(unsigned long, unsigned long, void *);
extern void work_with_active_regions(int nid, work_fn_t work_fn, void *data);

extern unsigned long highest_memmap_pfn;
extern int after_bootmem;

#define PAGE_ALIGN(addr) ALIGN(addr, PAGE_SIZE)

/* 2*/
#define ZONES_WIDTH ZONES_SHIFT
/* 0 */
#define NODES_WIDTH NODES_SHIFT

/* CONFIG_SPARSEMEM is not defined */
#define SECTIONS_WIDTH 0
/* 32 */
#define SECTIONS_PGOFF ((sizeof(unsigned long) * 8) - SECTIONS_WIDTH)
/* 32 */
#define NODES_PGOFF    (SECTIONS_PGOFF - NODES_WIDTH)
/* 30 */
#define ZONES_PGOFF    (NODES_PGOFF - ZONES_WIDTH)
/* 2 */
#define ZONEID_SHIFT   (NODES_SHIFT + ZONES_SHIFT)
/* 30 */
#define ZONEID_PGOFF   ((NODES_PGOFF < ZONES_PGOFF) ? NODES_PGOFF : ZONES_PGOFF)
/* 30 */
#define ZONEID_PGSHIFT (ZONEID_PGOFF * (ZONEID_SHIFT != 0))

/* 0 */
#define NODES_PGSHIFT (NODES_PGOFF * (NODES_WIDTH != 0))
/* 30 */
#define ZONES_PGSHIFT (ZONES_PGOFF * (ZONES_WIDTH != 0))
/* 0x00000003 */
#define ZONES_MASK ((1UL << ZONES_WIDTH) - 1)
/* 0x00000000 */
#define NODES_MASK ((1UL << NODES_WIDTH) - 1)
/* 0x00000003 */
#define ZONEID_MASK ((1UL << ZONEID_SHIFT) - 1)
static inline void set_page_zone(struct page *page, enum zone_type zone)
{
    page->flags &= ~(ZONES_MASK << ZONES_PGSHIFT);
	page->flags |= (zone & ZONES_MASK) << ZONES_PGSHIFT;
}

static inline void set_page_node(struct page *page, unsigned long node)
{
    page->flags &= ~(NODES_MASK << NODES_PGSHIFT);
	page->flags |= (node & NODES_MASK) << NODES_PGSHIFT;
}

static inline void set_page_links(struct page *page, enum zone_type zone, unsigned long node, unsigned long pfn)
{
    set_page_zone(page, zone);
	set_page_node(page, node);
}

static inline void init_page_count(struct page *page)
{
    atomic_set(&page->_count, 1);
}

static inline void reset_page_mapcount(struct page *page)
{
    atomic_set(&(page)->_mapcount, -1);
}

static inline int page_mapcount(struct page *page)
{
    return atomic_read(&(page)->_mapcount) + 1;
}

/* always return 0 */
static inline int page_to_nid (const struct page *page)
{
    return (page->flags >> NODES_PGSHIFT) & NODES_MASK;
}

static inline enum zone_type page_zonenum(const struct page *page)
{
    return (page->flags >> ZONES_PGSHIFT) & ZONES_MASK;
}

static inline struct zone *page_zone(const struct page *page)
{
    return &NODE_DATA(page_to_nid(page))->node_zones[page_zonenum(page)];
}

static inline int page_zone_id(struct page *page)
{
    return (page->flags >> ZONEID_PGSHIFT) & ZONEID_MASK;
}

#define PAGE_MAPPING_ANON    1
#define PAGE_MAPPING_KSM     2
#define PAGE_MAPPING_FLAGS   (PAGE_MAPPING_ANON | PAGE_MAPPING_KSM)

static inline int PageAnon(struct page *page)
{
    return ((unsigned long)page->mapping & PAGE_MAPPING_ANON) != 0;
}

static inline int compound_order(struct page *page)
{
    if (!PageHead(page))
		return 0;
	return (unsigned long)page[1].lru.prev;
}

static inline void set_compound_order(struct page *page, unsigned long order)
{
    page[1].lru.prev = (void *)order;
}

typedef void compound_page_dtor(struct page *);

static inline compound_page_dtor *get_compound_page_dtor(struct page *page)
{
    return (compound_page_dtor *)page[1].lru.next;
}

static inline void set_compound_page_dtor(struct page *page, compound_page_dtor *dtor)
{
    page[1].lru.next = (void *)dtor;
}

#define PAGE_BUDDY_MAPCOUNT_VALUE (-128)

static inline int PageBuddy(struct page *page)
{
    return atomic_read(&page->_mapcount) == PAGE_BUDDY_MAPCOUNT_VALUE;
}

static inline void __ClearPageBuddy(struct page *page)
{
    atomic_set(&page->_mapcount, -1);
}

static inline void __SetPageBuddy(struct page *page)
{
	atomic_set(&page->_mapcount, PAGE_BUDDY_MAPCOUNT_VALUE);
}

#define page_private(page) ((page)->private)
#define set_page_private(page, v) ((page)->private = (v))

/* zone->lock is already acquired when use these */
static inline unsigned long page_order(struct page *page)
{
    return page_private(page);
}

static void *lowmem_page_address(const struct page *page)
{
    return __va(PFN_PHYS(page_to_pfn(page)));
}

static inline struct page *compound_head(struct page *page)
{
    if (PageTail(page))
		return page->first_page;
	return page;
}

#define page_address(page) lowmem_page_address(page)

static inline struct page *virt_to_head_page(const void *x)
{
    struct page *page = virt_to_page(x);
	return compound_head(page);
}

void put_page(struct page *page);

static inline int put_page_testzero(struct page *page)
{
    return atomic_dec_and_test(&page->_count);
}

static inline void kernel_map_pages(struct page *page, int numpages, int enable) {}
#endif
