#ifndef _XC_KMEMCHECK_H
#define _XC_KMEMCHECK_H

#define kmemcheck_enabled 0
static inline void kmemcheck_alloc_shadow(struct page *page, int order, gfp_t flags, int node) {}
static inline void kmemcheck_free_shadow(struct page *page, int order) {}

static inline void kmemcheck_mark_uninitialized_pages(struct page *p, unsigned int n) {}

static inline void kmemcheck_mark_unallocated_pages(struct page *p, unsigned int n) {}


#endif
