#ifndef _XC_SLUB_DEF_H
#define _XC_SLUB_DEF_H

#include <Xc/types.h>
#include <Xc/spinlock.h>
#include <Xc/gfp.h>
#include <Xc/kernel.h>

struct kmem_cache_cpu {
    void **freelist;
	unsigned long tid;
	struct page *page;
	int node;
};

struct kmem_cache_node {
    spinlock_t list_lock;
	unsigned long nr_partial;
	struct list_head partial;
};

struct kmem_cache_order_objects {
    unsigned long x;
};

struct kmem_cache {
    struct kmem_cache_cpu *cpu_slab;
	unsigned long flags;
	unsigned long min_partial;
	int size;
	int objsize;
	int offset;
	struct kmem_cache_order_objects oo;

	struct kmem_cache_order_objects max;
	struct kmem_cache_order_objects min;
    gfp_t allocflags;
	int refcount;
	void (*ctor)(void *);
	int inuse;
	int align;
	int reserved;
	const char *name;
	struct list_head list;

	struct kmem_cache_node *node[MAX_NUMNODES];
};

#define KMALLOC_MIN_SIZE 8
/* = log2(KMALLOC_MIN_SIZE) */
#define KMALLOC_SHIFT_LOW 3

#define SLUB_MAX_SIZE (2 * PAGE_SIZE)
#define SLUB_PAGE_SHIFT (PAGE_SHIFT + 2)

#define SLUB_DMA __GFP_DMA

extern struct kmem_cache *kmalloc_caches[SLUB_PAGE_SHIFT];

static inline void *kmalloc_order(size_t size, gfp_t flags, unsigned int order)
{
    void *ret = (void *)__get_free_pages(flags | __GFP_COMP, order);
	/* kmemleak_alloc(ret, size, 1, flags); */
	return ret;
}

static inline void *kmalloc_order_trace(size_t size, gfp_t flags, unsigned int order)
{
    return kmalloc_order(size, flags, order);
}

static inline void *kmalloc_large(size_t size, gfp_t flags)
{
    unsigned int order = get_order(size);
	return kmalloc_order_trace(size, flags, order);
}

static inline int kmalloc_index(size_t size)
{
    if (!size)
		return 0;
	if (size <= KMALLOC_MIN_SIZE)
		return KMALLOC_SHIFT_LOW;
	if (KMALLOC_MIN_SIZE <= 32 && size > 64 && size <= 96)
		return 1;
	if (KMALLOC_MIN_SIZE <= 64 && size > 128 && size <= 192)
		return 2;
	
	if (size <= 8) return 3;
	if (size <= 16) return 4;
	if (size <= 32) return 5;
	if (size <= 64) return 6;
	if (size <= 128) return 7;
	if (size <= 256) return 8;
	if (size <= 512) return 9;
	if (size <= 1024) return 10;
	if (size <= 2 * 1024) return 11;
	if (size <= 4 * 1024) return 12;
	return -1;
}

static inline struct kmem_cache *kmalloc_slab(size_t size)
{
    int index = kmalloc_index(size);
	if (index == 0)
		return NULL;
	return kmalloc_caches[index];
}

void *kmem_cache_alloc(struct kmem_cache *s, gfp_t flags);
void *__kmalloc(size_t size, gfp_t flags);

static inline void *kmem_cache_alloc_trace(struct kmem_cache *s, gfp_t gfpflags, 
		                                   size_t size)
{
    return kmem_cache_alloc(s, gfpflags);
}

static inline void *kmalloc(size_t size, gfp_t flags)
{
    if (__builtin_constant_p(size)) {
        if (size > SLUB_MAX_SIZE)
			return kmalloc_large(size, flags);
		
		if (!(flags & SLUB_DMA)) {
            struct kmem_cache *s = kmalloc_slab(size);

			if (!s)
				return ZERO_SIZE_PTR;
			return kmem_cache_alloc_trace(s, flags, size);
		}
	}
	return __kmalloc(size, flags);
}

#endif
