#include <Xc/slab.h>
#include <Xc/threads.h>
#include <Xc/thread_info.h>
#include <Xc/spinlock.h>
#include <Xc/bit_spinlock.h>
#include <Xc/mm.h>
#include <Xc/percpu.h>
#include <Xc/cpumask.h>
#include <Xc/errno.h>
#include <Xc/irqflags.h>
#include <Xc/kmemcheck.h>
#include <Xc/vmstat.h>
#include <Xc/poison.h>
#include <Xc/current.h>
#include <Xc/rcupdate.h>
#include <Xc/cpumask.h>
#include <Xc/nodemask.h>
#include <Xc/swap.h>
#include <Xc/sched.h>
#include <Xc/string.h>
#include <asm/cache.h>
#include <asm/processor.h>
#include <asm/cmpxchg_32.h>
#include <stddef.h>

static enum {
    DOWN,
	PARTIAL,
	UP,
	SYSFS
} slab_state = DOWN;

static LIST_HEAD(slab_caches);

#define SLAB_DEBUG_FLAGS (SLAB_RED_ZONE | SLAB_POISON | SLAB_STORE_USER |    \
		SLAB_TRACE | SLAB_DEBUG_FREE)

#define __OBJECT_POSION 0x80000000UL
#define __CMPXCHG_DOUBLE 0x40000000UL

#define MIN_PARTIAL 5
#define MAX_PARTIAL 10

#define DEBUG_DEFAULT_FLAGS (SLAB_DEBUG_FREE | SLAB_RED_ZONE | SLAB_POISON | SLAB_STORE_USER)

#define DEBUG_METADATA_FLAGS (SLAB_RED_ZONE | SLAB_POISON | SLAB_STORE_USER)

#define OO_SHIFT 16
#define OO_MASK ((1 << OO_SHIFT) - 1)
#define MAX_OBJS_PER_PAGE 0x7FFF

static int kmem_size = sizeof(struct kmem_cache);

#define need_reserve_slab_rcu (sizeof(((struct page *)NULL)->lru) < sizeof(struct rcu_head))

static int slub_min_order;
static int slub_max_order = PAGE_ALLOC_COSTLY_ORDER;
static int slub_min_objects;

struct kmem_cache *kmalloc_caches[SLUB_PAGE_SHIFT];

static struct kmem_cache *kmem_cache;
static struct kmem_cache *kmalloc_dma_caches[SLUB_PAGE_SHIFT];

static inline void *get_freepointer(struct kmem_cache *s, void *object)
{
    return *(void **)(object + s->offset);
}

static inline void *get_freepointer_safe(struct kmem_cache *s, void *object)
{
    void *p;
	p = get_freepointer(s, object);
	return p;
}

static inline void set_freepointer(struct kmem_cache *s, void *object, void *fp)
{
    *(void **)(object + s->offset) = fp;
}

static inline int oo_order(struct kmem_cache_order_objects x)
{
    return x.x >> OO_SHIFT;
}

static inline int oo_objects(struct kmem_cache_order_objects x)
{
    return x.x & OO_MASK;
}

static inline int order_objects(int order, unsigned long size, int reserved)
{
    return ((PAGE_SIZE << order) - reserved) / size;	
}

static inline struct kmem_cache_order_objects oo_make(int order, unsigned long size, int reserved)
{
    struct kmem_cache_order_objects x = {
		(order << OO_SHIFT) + order_objects(order, size, reserved)
	};

	return x;
}

#define for_each_object(__p, __s, __addr, __objects)    \
	for (__p = (__addr); __p < (__addr) + (__objects) * (__s)->size; __p += (__s)->size)

static inline int node_match(struct kmem_cache_cpu *c, int node)
{
    return 1;
}

/*
static inline int kmem_cache_debug(struct kmem_cache *s)
{
    return 0;
}
*/

static void slab_out_of_memory(struct kmem_cache *s, gfp_t gfpflags, int nid)
{
    
}

static inline int alloc_debug_processing(struct kmem_cache *s, struct page *page, 
		                                 void *object, unsigned long addr) 
{
    return 0;
}

#define disable_higher_order_debug 0

static void remove_full(struct kmem_cache *s, struct page *page) {}

static inline void add_full(struct kmem_cache *s, struct kmem_cache_node *n, struct page *page) {}


static inline void add_partial(struct kmem_cache_node*n, struct page *page, int tail)
{
    n->nr_partial++;
	if (tail)
		list_add_tail(&page->lru, &n->partial);
	else
		list_add(&page->lru, &n->partial);
}

static inline void remove_partial(struct kmem_cache_node *n, struct page *page)
{
    list_del(&page->lru);
	n->nr_partial--;
}

static inline void inc_slabs_node(struct kmem_cache *s, int node, int objects) {}
static inline void dec_slabs_node(struct kmem_cache *s, int node, int objects) {}
static inline int slab_pre_alloc_hook(struct kmem_cache *s, gfp_t flags) { return 0; }
static inline void slab_post_alloc_hook(struct kmem_cache *s, gfp_t flags, void *object) {}
static inline void slab_lock(struct page *page)
{
    bit_spin_lock(PG_locked, &page->flags);
}

static inline void slab_unlock(struct page *page)
{
    __bit_spin_unlock(PG_locked, &page->flags);
}



static inline bool __cmpxchg_double_slab(struct kmem_cache *s, struct page *page, 
		                                 void *freelist_old, unsigned long counters_old, 
										 void *freelist_new, unsigned long counters_new,
										 const char *n)
{
    if (s->flags & __CMPXCHG_DOUBLE) {
        if(cmpxchg_double(&page->freelist, freelist_old, counters_old, 
					      freelist_new, counters_new))
			return 1;
	} else {
        slab_lock(page);
		if (page->freelist == freelist_old && page->counters == counters_old) {
            page->freelist = freelist_new;
			page->counters = counters_new;
			slab_unlock(page);
			return 1;
		}
		slab_unlock(page);
	}
	
	cpu_relax();
	
	/* stat(s, CMPXCHG_DOUBLE_FAIL); */
	return 0;
}

static inline bool cmpxchg_double_slab(struct kmem_cache *s, struct page *page, 
		                               void *freelist_old, unsigned long counters_old,
									   void *freelist_new, unsigned long counters_new, const char *n)
{
    if (s->flags & __CMPXCHG_DOUBLE) {
        if (cmpxchg_double(&page, freelist_old, counters_old, freelist_new, counters_new))
			return 1;
	} else {
        unsigned long flags;

		local_irq_save(flags);
		slab_lock(page);
		if (page->freelist == freelist_old && page->counters == counters_old) {
            page->freelist = freelist_new;
			page->counters = counters_new;
			slab_unlock(page);
			local_irq_restore(flags);
			return 1;
		}

		slab_unlock(page);
		local_irq_restore(flags);
	}
	cpu_relax();
	/* stat(s, CMPXCHG_DOUBLE_FAIL); */
	return 0;
}

static inline int acquire_slab(struct kmem_cache *s, struct kmem_cache_node *n, 
		                       struct page *page)
{
    void *freelist;
	unsigned long counters;
	struct page new;

	do {
        freelist = page->freelist;
		counters = page->counters;
		new.counters = counters;
		new.inuse = page->objects;

		new.frozen = 1;
	} while (!__cmpxchg_double_slab(s, page, freelist, counters, NULL, new.counters, 
				                    "lock and freeze"));

	remove_partial(n, page);

	if (freelist) {
        this_cpu_write(s->cpu_slab->freelist, freelist);
		this_cpu_write(s->cpu_slab->page, page);
		this_cpu_write(s->cpu_slab->node, page_to_nid(page));
		return 1;
	} else {
        return 0;
	}
}

static struct page *get_partial_node(struct kmem_cache *s, struct kmem_cache_node *n)
{
    struct page *page;

	if (!n || !n->nr_partial)
		return NULL;

	spin_lock(&n->list_lock);
    list_for_each_entry(page, &n->partial, lru)
		if (acquire_slab(s, n, page))
			goto out;

	page = NULL;
out:
	spin_unlock(&n->list_lock);
	return page;
}

static inline struct kmem_cache_node *get_node(struct kmem_cache *s, int node)
{
    return s->node[node];
}

static inline struct page *get_any_partial(struct kmem_cache *s, gfp_t flags)
{
    return NULL;
}

static struct page *get_partial(struct kmem_cache *s, gfp_t flags, int node)
{
    struct page *page;
	int searchnode = (node == NUMA_NO_NODE) ? numa_node_id() : node;

	page = get_partial_node(s, get_node(s, searchnode));
	if (page || node != NUMA_NO_NODE)
		return page;

	return get_any_partial(s, flags);
}


int slab_is_available(void)
{
    return slab_state >= UP;
}

#define TID_STEP roundup_pow_of_two(CONFIG_NR_CPUS)

static inline unsigned long next_tid(unsigned long tid)
{
    return tid + TID_STEP;
}

static inline unsigned long kmem_cache_flags(unsigned long objsize, unsigned long flags, 
		                                     const char *name, void (*ctor)(void *))
{
    return flags;
}

static unsigned long calculate_alignment(unsigned long flags, unsigned long align, 
		                                 unsigned long size)
{
    if (flags & SLAB_HWCACHE_ALIGN) {
        unsigned long ralign = cache_line_size();
		while (size <= (ralign >> 1)) /* By harvey, Don't both align and ralign be the power of 2? */
			ralign >>= 1;
		align = max(align, ralign);
	}

	if (align < ARCH_SLAB_MINALIGN)
		align = ARCH_SLAB_MINALIGN;

	return ALIGN(align, sizeof(void *));
}


static inline int slab_order(int size, int min_objects, int max_order, 
		                     int fract_leftover, int reserved)
{
    int order;
	int rem;
	int min_order = slub_min_order;
	
	if (order_objects(min_order, size, reserved) > MAX_OBJS_PER_PAGE)
		return get_order(size * MAX_OBJS_PER_PAGE) - 1;
    
	for (order = max(min_order, fls(min_objects * size - 1) - PAGE_SHIFT);
		 order <= max_order; order++) {
        unsigned long slab_size = PAGE_SIZE << order;
		if (slab_size < min_objects * size + reserved)
			continue;

		rem = (slab_size - reserved) % size;
		if (rem <= slab_size / fract_leftover)
			break;
	}

	return order;
}

static inline int calculate_order(int size, int reserved)
{
    int order;
	int min_objects;
	int fraction;
	int max_objects;

	min_objects = slub_min_objects;
	if (!min_objects)
		min_objects = 4 * (fls(nr_cpu_ids) + 1);
	max_objects = order_objects(slub_max_order, size, reserved);
	min_objects = min(min_objects, max_objects);

	while (min_objects > 1) {
        fraction = 16;
		while (fraction >= 4) {
            order = slab_order(size, min_objects, slub_max_order, fraction, reserved);
			if (order <= slub_max_order)
				return order;
			fraction >>= 1;
		}
		min_objects--;
	}

	order = slab_order(size, 1, slub_max_order, 1, reserved);
	if (order <= slub_max_order)
		return order;

	order = slab_order(size, 1, MAX_ORDER, 1, reserved);
	if (order < MAX_ORDER)
		return order;
	return -ENOSYS;
}


static int calculate_sizes(struct kmem_cache *s, int forced_order)
{
    unsigned long flags = s->flags;
	unsigned long size = s->objsize;
	unsigned long align = s->align;
	int order;

	size = ALIGN(size, sizeof(void *));
	s->inuse = size;
	
	if (((flags & (SLAB_DESTROY_BY_RCU | SLAB_POISON)) || s->ctor)) {
        s->offset = size;
		size += sizeof(void *);
	}

	align = calculate_alignment(flags, align, s->objsize);
	s->align = align;

	size = ALIGN(size, align);
	s->size = size;

	if (forced_order >= 0)
		order = forced_order;
	else
		order = calculate_order(size, s->reserved);

	if (order < 0)
		return 0;

	s->allocflags = 0;
	if (order)
		s->allocflags |= __GFP_COMP;

	if (s->flags & SLAB_CACHE_DMA)
		s->allocflags |= SLUB_DMA;

	if (s->flags & SLAB_RECLAIM_ACCOUNT)
		s->allocflags |= __GFP_RECLAIMABLE;

	s->oo = oo_make(order, size, s->reserved);
	s->min = oo_make(get_order(size), size, s->reserved);
    if(oo_objects(s->oo) > oo_objects(s->max))
		s->max = s->oo;

    return !!oo_objects(s->oo);
}

static void set_min_partial(struct kmem_cache *s ,unsigned long min)
{
    if (min < MIN_PARTIAL)
		min = MIN_PARTIAL;
	 else if (min > MAX_PARTIAL)
		min = MAX_PARTIAL;
	s->min_partial = min;
}

static inline struct page *alloc_slab_page(gfp_t flags, int node, struct kmem_cache_order_objects oo)
{
    int order = oo_order(oo);
	flags |= __GFP_NOTRACK;
	if (node == NUMA_NO_NODE)
		return alloc_pages(flags, order);
	else
		return alloc_pages_exact_node(node, flags, order);
}

static struct page *allocate_slab(struct kmem_cache *s, gfp_t flags, int node)
{
    struct page *page;
	struct kmem_cache_order_objects oo = s->oo;
	gfp_t alloc_gfp;

	flags &= gfp_allowed_mask;

	if (flags & __GFP_WAIT)
		local_irq_enable();

	flags |= s->allocflags;

    alloc_gfp = (flags | __GFP_NOWARN | __GFP_NORETRY) & ~__GFP_NOFAIL;

	page = alloc_slab_page(alloc_gfp, node, oo);
	if (!page) {
        oo = s->min;
		page = alloc_slab_page(flags, node, oo);

		/*
		if (page)
			stat(s, ORDER_FALLBACK);
			*/
	}

	if (flags & __GFP_WAIT)
		local_irq_disable();
	if (!page)
		return NULL;

	if (kmemcheck_enabled && !(s->flags & (SLAB_NOTRACK | DEBUG_DEFAULT_FLAGS))) {
        int pages = 1 << oo_order(oo);
		kmemcheck_alloc_shadow(page, oo_order(oo), flags, node);

		if (s->ctor)
			kmemcheck_mark_uninitialized_pages(page, pages);
		else
			kmemcheck_mark_unallocated_pages(page, pages);
	}

	page->objects = oo_objects(oo);
	mod_zone_page_state(page_zone(page), 
			            (s->flags & SLAB_RECLAIM_ACCOUNT) ? 
						 NR_SLAB_RECLAIMABLE : NR_SLAB_UNRECLAIMABLE, 1 << oo_order(oo));
	return page;
}


static void setup_object(struct kmem_cache *s, struct page *page, void *object)
{
    /* setup_object_debug(s, page, object); */
	if (s->ctor)
		s->ctor(object);
}

static struct page *new_slab(struct kmem_cache *s, gfp_t flags, int node)
{
    struct page *page;
	void *start;
	void *last;
	void *p;

	page = allocate_slab(s, flags & (GFP_RECLAIM_MASK | GFP_CONSTRAINT_MASK), node);
    if (!page)
		goto out;

	inc_slabs_node(s, page_to_nid(page), page->objects);
	page->slab = s;
	page->flags |= 1 << PG_slab;

	start = page_address(page);

	if (s->flags & SLAB_POISON)
		memset(start, POISON_INUSE, PAGE_SIZE << compound_order(page));

	last = start;
	for_each_object(p, s, start, page->objects) {
        setup_object(s, page, last);
		set_freepointer(s, last, p);
		last = p;
	}

	setup_object(s, page, last);
	set_freepointer(s, last, NULL);

	page->freelist = start;
	page->inuse = 0;
	page->frozen = 1;
out:
	return page;
}

static void init_kmem_cache_node(struct kmem_cache_node *n, struct kmem_cache *s)
{
    n->nr_partial = 0;
	spin_lock_init(&n->list_lock);
	INIT_LIST_HEAD(&n->partial);

	/*
	atomic_long_set(&n->nr_slabs, 0);
	atomic_long_set(&n->total_objects, 0);
	INIT_LIST_HEAD(&n->full);
	*/
}



static void __free_slab(struct kmem_cache *s, struct page *page)
{
    int order = compound_order(page);
	int pages = 1 << order;

	/*
	if (kmem_cache_debug(s)) {
        
	}
	*/

	/* kmemcheck_free_shadow(page, compound_order(page)); */
	/* mod_zone_page_state(page_zone(page), (s->flags & SLAB_RECLAIM_ACCOUNT) ? 
			                             NR_SLAB_RECLAIMABLE : NR_SLAB_UNRECLAIMABLE, 
						, -pages); */

	__ClearPageSlab(page);
	reset_page_mapcount(page);
	if (current->reclaim_state)
		current->reclaim_state->reclaimed_slab += pages;
	__free_pages(page, order);
}

static void free_slab(struct kmem_cache *s, struct page *page)
{
    if (s->flags & SLAB_DESTROY_BY_RCU) {
		/*
        struct rcu_head *head;

		if (need_reserve_slab_rcu) {
            int order = compound_order(page);
			int offset = (PAGE_SIZE << order) - s->reserved;

			head = page_address(page) + offset;
		} else {
            head = (void *)&page->lru;
		}
		call_rcu(head, rcu_free_slab);
		*/
	} else
		__free_slab(s, page);
}

static void discard_slab(struct kmem_cache *s, struct page *page)
{
    dec_slabs_node(s, page_to_nid(page), page->objects);
	free_slab(s, page);
}

static void deactivate_slab(struct kmem_cache *s, struct kmem_cache_cpu *c)
{
    enum slab_modes {M_NONE, M_PARTIAL, M_FULL, M_FREE};
    struct page *page = c->page;
	struct kmem_cache_node *n = get_node(s, page_to_nid(page));
	int lock = 0;

	enum slab_modes l = M_NONE, m = M_NONE;
	void *freelist;
	void *nextfree;
	int tail = 0;
	struct page new;
	struct page old;

	if (page->freelist) {
        /* stat(s, DEACTIVATE_REMOTE_FREES); */
		tail = 1;
	}

    c->tid = next_tid(c->tid);
	c->page = NULL;
	freelist = c->freelist;
	c->freelist = NULL;

	while (freelist && (nextfree = get_freepointer(s, freelist))) {
        void *prior;
		unsigned long counters;

		do {
			prior = page->freelist;
			counters = page->counters;
			set_freepointer(s, freelist, prior);
			new.counters = counters;
			new.inuse--;
		} while (!__cmpxchg_double_slab(s, page, prior, counters, freelist, new.counters, 
					"drain percpu freelist"));
		freelist = nextfree;
	}

redo:
	old.freelist = page->freelist;
	old.counters = page->counters;
	new.counters = old.counters;
	if (freelist) {
        new.inuse--;
		set_freepointer(s, freelist, old.freelist);
		new.freelist = freelist;
	} else
		new.freelist = old.freelist;

	new.frozen = 0;
	
	if (!new.inuse && n->nr_partial > s->min_partial)
		m = M_FREE;
	else if (new.freelist) {
        m = M_PARTIAL;
		if (!lock) {
            lock = 1;
			spin_lock(&n->list_lock);
		}
	} else {
        m = M_FULL;
		/*
		if (kmem_cache_debug(s) && !lock) {
            lock = 1;
			spin_lock(&n->list_lock);
		}
		*/
	}

	if (l != m) {
        if (l == M_PARTIAL)
			remove_partial(n, page);
		else if (l == M_FULL)
			remove_full(s, page);

		if (m == M_PARTIAL) {
            add_partial(n, page, tail);
			/* stat(s, tial ? DEACTIVATE_TO_TAIL : DEACTIVATE_TO_HEAD); */
		} else if (m == M_FULL) {
            /* stat(s, DEACTIVATE_FULL); */
			add_full(s, n, page);
		}
	}

	l = m;

	if (!__cmpxchg_double_slab(s, page, old.freelist, old.counters, new.freelist, new.counters, 
				"unfreezing slab"))
		goto redo;

	if (lock)
		spin_lock(&n->list_lock);
	if (m == M_FREE) {
        /* stat(s, DEACTIVATE_EMPTY); */
		discard_slab(s, page);
		/* stat(s, FREE_SLAB); */
	}
}

static inline void flush_slab(struct kmem_cache *s, struct kmem_cache_cpu *c)
{
    /* stat(s, CPUSLAB_FLUSH); */
	deactivate_slab(s, c);
}


static void *__slab_alloc(struct kmem_cache *s, gfp_t gfpflags, int node, 
		                  unsigned long addr, struct kmem_cache_cpu *c)
{
    void **object;
	struct page *page;
	unsigned long flags;
	struct page new;
	unsigned long counters;

	local_irq_save(flags);

	c = this_cpu_ptr(s->cpu_slab); /* By harvey, Reload due to the possible preemption */

	gfpflags &= ~__GFP_ZERO;
	page = c->page;
	if (!page)
		goto new_slab;

	if (!node_match(c, node)) {
        /* stat(s, ALLOC_NODE_MISMATCH); */
		deactivate_slab(s, c);
		goto new_slab;
	}

	/* stat(s, ALLOC_SLOWPATH); */
	do {
        object = page->freelist;
		counters = page->counters;
		new.counters = counters;

		new.inuse = page->objects;
		new.frozen = object != NULL;
	} while (!__cmpxchg_double_slab(s, page, object, counters, NULL, new.counters, 
				                    "__slab_alloc"));

	if (!object) {
        c->page = NULL;
		/* stat(s, DEACTIVATE_BYPASS); */
		goto new_slab;
	}

	/* stat(s, ALLOC_REFILL); */

load_freelist:
	c->freelist = get_freepointer(s, object);
	c->tid = next_tid(c->tid);
	local_irq_restore(flags);
	return object;

new_slab:
	page = get_partial(s, gfpflags, node);
	if (page) {
        /* stat(s, ALLOC_FROM_PARTIAL); */
		object = c->freelist;
        
		/*
		if (kmem_cache_debug(s))
			goto debug;
		*/
		goto load_freelist;
	}

	page = new_slab(s, gfpflags, node);
	if (page) {
        c = __this_cpu_ptr(s->cpu_slab);
		if (c->page)
			flush_slab(s, c);

		object = page->freelist;
		page->freelist = NULL;
		page->inuse = page->objects;

		/* stat(s, ALLOC_SLAB); */
		c->node = page_to_nid(page);
		c->page = page;
        
		/*
		if (kmem_cache_debug(s))
			goto debug;
		*/
		goto load_freelist;
	}

	if (!(gfpflags & __GFP_NOWARN) && printk_ratelimit())
		slab_out_of_memory(s, gfpflags, node);
	local_irq_restore(flags);
	return NULL;

debug:
	if (!object || !alloc_debug_processing(s, page, object, addr))
		goto new_slab;

	c->freelist = get_freepointer(s, object);
	deactivate_slab(s, c);
	c->page = NULL;
	c->node = NUMA_NO_NODE;
	local_irq_restore(flags);
	return object;
}

static inline void *slab_alloc(struct kmem_cache *s, gfp_t gfpflags, int node, 
		                       unsigned long addr)
{
    void **object;
	struct kmem_cache_cpu *c;
	unsigned long tid;

	if (slab_pre_alloc_hook(s, gfpflags))
		return NULL;

redo:
	c = __this_cpu_ptr(s->cpu_slab);
    tid = c->tid;
	barrier();

	object = c->freelist;
	if (!object || !node_match(c, node))
		object = __slab_alloc(s, gfpflags, node, addr, c);
	else {
        if (!irqsafe_cpu_cmpxchg_double(s->cpu_slab->freelist, s->cpu_slab->tid, object, 
					                    tid, get_freepointer_safe(s, object), 
										next_tid(tid))) {
            /* note_cmpxchg_failure("slab_alloc", s, tid); */
			goto redo;
		}
		/* stat(s, ALLOC_FASTPATH); */
	}
	if ((gfpflags & __GFP_ZERO) && object)
		memset(object, 0, s->objsize);

	slab_post_alloc_hook(s, gfpflags, object);

	return object;
}

static u8 size_index[24] = {
    3,
	4,
	5,
	5,
	6,
	6,
	6,
	6,
	1,
	1,
	1,
	1,
	7,
	7,
	7,
	7,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
};

static inline int size_index_elem(size_t bytes)
{
    return (bytes - 1) / 8;
}

static struct kmem_cache *get_slab(size_t size, gfp_t flags)
{
    int index;

	if (size <= 192) {
        if (!size)
			return ZERO_SIZE_PTR;
		index = size_index[size_index_elem(size)];
	} else
        index = fls(size - 1);

	if (flags & SLUB_DMA)
		return kmalloc_dma_caches[index];

	return kmalloc_caches[index];
}

void *__kmalloc(size_t size, gfp_t flags)
{
    struct kmem_cache *s;
	void *ret;

	if (size > SLUB_MAX_SIZE)
		return kmalloc_large(size, flags);

	s = get_slab(size, flags);
	if (ZERO_OR_NULL_PTR(s))
		return s;

	ret = slab_alloc(s, flags, NUMA_NO_NODE, _RET_IP_);
	/* trace_kmalloc(_RET_IP_, ret, size, s->size, flags); */
	return ret;
}

void *kmem_cache_alloc(struct kmem_cache *s, gfp_t gfpflags)
{
    void *ret = slab_alloc(s, gfpflags, NUMA_NO_NODE, _RET_IP_);
	
    /* trace_kmem_cache_alloc(_RET_IP_, ret, s->objsize, s->size, gfpflags); */
	return ret;
}


static void __slab_free(struct kmem_cache *s, struct page *page, void *x, 
		                unsigned long addr)
{
    void *prior;
	void **object = (void *)x;
	int was_frozen;
	int inuse;
	struct page new;
	unsigned long counters;
	struct kmem_cache_node *n = NULL;
	unsigned long uninitialized_var(flags);

	/* stat(s, FREE_SLOWPATH); */
	/* if (kmem_cache_debug(s) && !free_debug_processing(s, page, x, addr))
		return;
    */
	do {
        prior = page->freelist;
		counters = page->counters;
		set_freepointer(s, object, prior);
		new.counters = counters;
		was_frozen = new.frozen;
		new.inuse--;
		if ((!new.inuse || !prior) && !was_frozen && !n) {
            n = get_node(s, page_to_nid(page));
			
			spin_lock_irqsave(&n->list_lock, flags);
		}
		inuse = new.inuse;
	} while (!cmpxchg_double_slab(s, page, prior, counters, object, new.counters, 
				                  "__slab_free"));

	if (!n) {
		/*
        if (was_frozen)
			stat(s, FREE_FROZEN);
		*/
		return;
	}

	if (was_frozen) {
		/* stat(s, FREE_FROZEN); */
	}
	else {
        if (!inuse && n->nr_partial > s->min_partial)
			goto slab_empty;

		if (!prior) {
            remove_full(s, page);
			add_partial(n, page, 1);
			/* stat(s, FREE_ADD_PARTIAL); */
		}
	}

	spin_unlock_irqrestore(&n->list_lock, flags);
	return;

slab_empty:
	if (prior) {
        remove_partial(n, page);
		/* stat(s, FREE_REMOVE_PARTIAL); */
	} else
		remove_full(s, page);

	spin_unlock_irqrestore(&n->list_lock, flags);
	/* stat(s, FREE_SLAB); */
	discard_slab(s, page);
}

static void slab_free(struct kmem_cache *s, struct page *page, void *x, 
		              unsigned long addr)
{
    void **object = (void *)x;
	struct kmem_cache_cpu *c;
	unsigned long tid;

	/* slab_free_hook(s, x); */
redo:
	c = __this_cpu_ptr(s->cpu_slab);
	tid = c->tid;
	barrier();

	if (page == c->page) {
        set_freepointer(s, object, c->freelist);
		if (!irqsafe_cpu_cmpxchg_double(s->cpu_slab->freelist, s->cpu_slab->tid, 
					                    c->freelist, tid, object, next_tid(tid))) {
			/* note_cmpxchg_failure("slab_free", s, tid); */
            goto redo;
		}
		/* stat(s, FREE_FASTPATH); */
	} else
		__slab_free(s, page, x, addr);
}

void kfree(const void *x)
{
    struct page *page;
	void *object = (void *)x;

	/* trace_kfree(_RET_IP_, x); */

	if (ZERO_OR_NULL_PTR(x))
		return;

	page = virt_to_head_page(x);
	if (!PageSlab(page)) {
		/* BUG_ON(!PageCompound(page)); */
        /* kmemleak_free(x); */
		put_page(page);
		return;
	}
	slab_free(page->slab, page, object, _RET_IP_);
}


void kmem_cache_free(struct kmem_cache *s, void *x)
{
    struct page *page;
	page = virt_to_head_page(x);
	slab_free(s, page, x, _RET_IP_);
	/* trace_kmem_cache_free(_RET_IP_, x); */
}

static struct kmem_cache *kmem_cache_node;

static void free_kmem_cache_nodes(struct kmem_cache *s)
{
    int node;

	for_each_node_state(node, N_NORMAL_MEMORY) {
        struct kmem_cache_node *n = s->node[node];

		if (n)
			kmem_cache_free(kmem_cache_node, n);
		s->node[node] = NULL;
	}
}


static void early_kmem_cache_node_alloc(int node)
{
    struct page *page;
	struct kmem_cache_node *n;

	/* BUG_ON(kmem_cache_node->size < sizeof(struct kmem_cache_node)); */
	page = new_slab(kmem_cache_node, GFP_NOWAIT, node);

	/* BUG_ON(!page); */
	if (page_to_nid(page) != node) {
        panic("SLUB: Unable to allocate memory from node");
	}

	n = page->freelist;
	/* BUG_ON(!n); */
	page->freelist = get_freepointer(kmem_cache_node, n);
	page->inuse++;
	page->frozen = 0;
	kmem_cache_node->node[node] = n;

	init_kmem_cache_node(n, kmem_cache_node);
	inc_slabs_node(kmem_cache_node, node, page->objects);

	add_partial(n, page, 0);
}


static int init_kmem_cache_nodes(struct kmem_cache *s)
{
    int node;

	for_each_node_state(node, N_NORMAL_MEMORY) {
        struct kmem_cache_node *n;
		
		if (slab_state == DOWN) {
            early_kmem_cache_node_alloc(node);
			continue;
		}

		n = kmem_cache_alloc_node(kmem_cache_node, GFP_KERNEL, node);

		if (!n) {
            free_kmem_cache_nodes(s);
			return 0;
		}

		s->node[node] = n;
		init_kmem_cache_node(n, s);
	}
	return 1;
}

static unsigned int init_tid(int cpu)
{
    return cpu;
}

void init_kmem_cache_cpus(struct kmem_cache *s)
{
	int cpu;
    for_each_possible_cpu(cpu)
		per_cpu_ptr(kmem_cache_node->cpu_slab, cpu)->tid = init_tid(cpu);
    
}
static inline int alloc_kmem_cache_cpus(struct kmem_cache *s)
{
	/*By harvey, initialize kmem_cache_cpu */
	unsigned long size;
	size = offsetof(struct kmem_cache, node) + nr_node_ids * sizeof(struct kmem_cache_node *);
	s->cpu_slab = (void *)s + ALIGN(size, 2 * sizeof(void *));
	/* end -- by harvey */
	if (!s->cpu_slab)
		return 0;
	init_kmem_cache_cpus(s);
    return 1;
}

static int kmem_cache_open(struct kmem_cache *s, const char *name, size_t size, 
		                   size_t align, unsigned long flags, void (*ctor)(void *))
{
    memset(s, 0, kmem_size);
	s->name = name;
	s->ctor = ctor;
	s->objsize = size;
	s->align = align;
	s->flags = kmem_cache_flags(size, flags, name, ctor);
	s->reserved = 0;

    if (need_reserve_slab_rcu && (s->flags & SLAB_DESTROY_BY_RCU))
		s->reserved = sizeof(struct rcu_head);

	if (!calculate_sizes(s, -1))
		goto error;

	if (disable_higher_order_debug) {
        if (get_order(s->size) > get_order(s->objsize)) {
            s->flags &= ~DEBUG_METADATA_FLAGS;
			s->offset = 0;
			if (!calculate_sizes(s, -1))
				goto error;
		}
	}

	/* CONFIG_CMPXCHG_DOUBLE is not defined */
	if (system_has_cmpxchg_double() && (s->flags & SLAB_DEBUG_FLAGS) == 0)
		s->flags |= __CMPXCHG_DOUBLE;

    set_min_partial(s, ilog2(s->size));
	s->refcount = 1;

	if (!init_kmem_cache_nodes(s))
		goto error;

	if (alloc_kmem_cache_cpus(s))
		return 1;

	free_kmem_cache_nodes(s);
error:
	if (flags & SLAB_PANIC)
		panic("Cannot create slab");

	return 0;
}

static void kmem_cache_bootstrap_fixup(struct kmem_cache *s)
{
    int node;
	list_add(&s->list, &slab_caches);
	s->refcount = -1;

	for_each_node_state(node, N_NORMAL_MEMORY) {
        struct kmem_cache_node *n = get_node(s, node);
		struct page *p;

		if (n) {
            list_for_each_entry(p, &n->partial, lru)
				p->slab = s;
		}
	}
}

static struct kmem_cache *create_kmalloc_cache(const char *name, int size, unsigned int flags)
{
    struct kmem_cache *s;
	s = kmem_cache_alloc(kmem_cache, GFP_NOWAIT);

	if (!kmem_cache_open(s, name, size, ARCH_KMALLOC_MINALIGN, flags, NULL))
		goto panic;
	list_add(&s->list, &slab_caches);
	return s;
panic:
	panic("Creation of kmalloc slab");
	return NULL;
}

void kmem_cache_init(void)
{
    int i;
	int caches = 0;
	struct kmem_cache *temp_kmem_cache;
	int order;
	struct kmem_cache *temp_kmem_cache_node;
	unsigned long kmalloc_size;

	/* By harvey, alloc memory for kmem_cache_cpu */
	kmalloc_size = offsetof(struct kmem_cache, node) + 
		nr_node_ids * sizeof(struct kmem_cache_node *);

	kmalloc_size = ALIGN(kmalloc_size, 2 * sizeof(void *));
	kmalloc_size = kmalloc_size + nr_cpu_ids * sizeof(struct kmem_cache_cpu);
    kmalloc_size = ALIGN(kmalloc_size, cache_line_size());

	order = get_order(2 * kmalloc_size);
	kmem_cache = (void *)__get_free_pages(GFP_NOWAIT, order);

	kmem_cache_node = (void *)kmem_cache + kmalloc_size;

	kmem_cache_open(kmem_cache_node, "kmem_cache_node", 
			        sizeof(struct kmem_cache_node), 0,
			        SLAB_HWCACHE_ALIGN | SLAB_PANIC, NULL);
    
	/*By harvey, initialize kmem_cache_cpu
	kmem_cache_node->cpu_slab = kmem_cache_node + kmalloc_size2;
    for_each_possible_cpu(cpu)
		per_cpu_ptr(kmem_cache_node->cpu_slab, cpu)->tid = cpu;
	end -- by harvey */

	/* hotplug_memory_notifier(slab_memory_callback, SLAB_CALLBACK_PRI); */

    slab_state = PARTIAL;
	temp_kmem_cache = kmem_cache;

	/* By harvey, alloc memory for kmem_cache_cpu at the tail, so size is kmalloc_size, instead of kmem_size*/
	kmem_cache_open(kmem_cache, "kmem_cache", kmalloc_size, 0, 
			        SLAB_HWCACHE_ALIGN | SLAB_PANIC, NULL);
	kmem_cache = kmem_cache_alloc(kmem_cache, GFP_NOWAIT);
	memcpy(kmem_cache, temp_kmem_cache, kmalloc_size);
    
	/*By harvey, relocate the per-cpu cpu_slab*/
	kmem_cache->cpu_slab = (void *)kmem_cache + ALIGN(kmem_size, 2 * sizeof(void *));

	temp_kmem_cache_node = kmem_cache_node;

	kmem_cache_node = kmem_cache_alloc(kmem_cache, GFP_NOWAIT);
	memcpy(kmem_cache_node, temp_kmem_cache_node, kmalloc_size);

	/*By harvey, relocate the per-cpu cpu_slab*/
	kmem_cache_node->cpu_slab = (void *)kmem_cache_node + ALIGN(kmem_size, 2 * sizeof(void *));

    kmem_cache_bootstrap_fixup(kmem_cache_node);
    caches++;
	kmem_cache_bootstrap_fixup(kmem_cache);
	caches++;

	free_pages((unsigned long)temp_kmem_cache, order);

	for (i = 8; i < KMALLOC_MIN_SIZE; i += 8) {
        int elem = size_index_elem(i);
		if (elem >= ARRAY_SIZE(size_index))
			break;
		size_index[elem] = KMALLOC_SHIFT_LOW;
	}

	if (KMALLOC_MIN_SIZE == 64) {
        for (i = 64  + 8; i <= 96; i += 8)
			size_index[size_index_elem(i)] = 7;
	} else if (KMALLOC_MIN_SIZE == 128) {
        for (i = 128 + 8; i <= 192; i += 8)
			size_index[size_index_elem(i)] = 8;
	}

	if (KMALLOC_MIN_SIZE <= 32) {
        kmalloc_caches[1] = create_kmalloc_cache("kmalloc-96", 96, 0);
		caches++;
	}

	if (KMALLOC_MIN_SIZE <= 64) {
        kmalloc_caches[2] = create_kmalloc_cache("kmalloc-192", 192, 0);
		caches++;
	}

	
	for (i = KMALLOC_SHIFT_LOW; i < SLUB_PAGE_SHIFT; i++) {
        kmalloc_caches[i] = create_kmalloc_cache("kmalloc", 1 << i, 0);
		caches++;
	}
	

	slab_state = UP;

	if (KMALLOC_MIN_SIZE <= 32) {
        /* kmalloc_caches[1]->name = kstrdup(kmalloc_caches[1]->name, GFP_NOWAIT); */
        kmalloc_caches[1]->name = NULL;
	}

	if (KMALLOC_MIN_SIZE <= 64) {
        /* kmalloc_caches[2]->name = kstrdup(kmalloc_caches[2]->name, GFP_NOWAIT); */
        kmalloc_caches[2]->name = NULL;
	}

	for (i = KMALLOC_SHIFT_LOW; i < SLUB_PAGE_SHIFT; i++) {
		/*
		 * char *s = kasprintf(GFP_NOWAIT, "kmalloc-%d", 1 << i);
         *
         * kmalloc_caches[i]->name = s;
		 */
	}

	/*
	for (i = 0; i < SLUB_PAGE_SHIFT; i++) {
        struct kmem_cache *s = kmalloc_caches[i];

		if (s && s->size) {
            // char *name = kasprintf(GFP_NOWAIT, "dma-kmalloc-%d", s->objsize);
			char *name = "dma-kmalloc";
            kmalloc_dma_caches[i] = create_kmalloc_cache(name, s->objsize, 
					                                     SLAB_CACHE_DMA);
		}
	}
	*/
}

void kmem_cache_init_late(void) {}
