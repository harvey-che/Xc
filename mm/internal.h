#ifndef _MM_INTERNAL_H
#define _MM_INTERNAL_H

#include <Xc/atomic.h>

#define ZONE_RECLAIM_NOSCAN -2
#define ZONE_RECLAIM_FULL   -1
#define ZONE_RECLAIM_SOME   0
#define ZONE_RECLAIM_SUCCESS 1

static inline void set_page_count(struct page *page, int v)
{
    atomic_set(&page->_count, v);
} 

static inline void set_page_refcounted(struct page *page)
{
    set_page_count(page, 1);
}

#endif

