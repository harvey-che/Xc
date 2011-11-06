#ifndef _XC_SLAB_H
#define _XC_SLAB_H

#include <Xc/gfp.h>
#include <stddef.h>

#define ZERO_SIZE_PTR ((void *)16)
#define ZERO_OR_NULL_PTR(x) ((unsigned long)(x) <= (unsigned long)ZERO_SIZE_PTR)


#define SLAB_DEBUG_FREE 0x00000100UL
#define SLAB_RED_ZONE   0x00000400UL
#define SLAB_POISON     0x00000800UL
#define SLAB_HWCACHE_ALIGN 0x00002000UL
#define SLAB_CACHE_DMA  0x00004000UL
#define SLAB_STORE_USER 0x00010000UL
#define SLAB_PANIC      0x00040000UL

#define SLAB_DESTROY_BY_RCU  0x00080000UL
#define SLAB_TRACE      0x00200000UL

#define SLAB_NOTRACK    0x00000000UL

#define SLAB_RECLAIM_ACCOUNT 0x00020000UL

#define ARCH_SLAB_MINALIGN __alignof__(unsigned long long)
#define ARCH_KMALLOC_MINALIGN __alignof__(unsigned long long)
#include <Xc/slub_def.h>


void kmem_cache_init(void);
void kmem_cache_init_late(void);

static void *kmalloc_node(size_t size, gfp_t flags, int node)
{
    return NULL;
}

int slab_is_available(void);

static inline void *kzalloc(size_t size, gfp_t flags)
{
    return kmalloc(size, flags | __GFP_ZERO);
}

static inline void *kzalloc_node(size_t size, gfp_t flags, int node)
{
    return kmalloc_node(size, flags | __GFP_ZERO, node);
}

static inline void *kmem_cache_alloc_node(struct kmem_cache *cachep, gfp_t flags, int nodes)
{
    return kmem_cache_alloc(cachep, flags);
}

void kmem_cache_init(void);
#endif
