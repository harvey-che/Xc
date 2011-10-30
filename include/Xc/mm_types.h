#ifndef _XC_MM_TYPES_H
#define _XC_MM_TYPES_H
#include <Xc/types.h>

struct address_space;
struct kmem_cache;

struct page {
    unsigned long flags;
	struct address_space *mapping;

	struct {
		union {
			pgoff_t index;
			void *freelist;
		};
        union {
            unsigned long counters;
			struct {
                union {
                    atomic_t _mapcount;
					struct {
                        unsigned inuse:16;
						unsigned objects:15;
						unsigned frozen:1;
					};
				};
				atomic_t _count;
			};
		};
	};

	struct list_head lru;

	union {
        unsigned long private;
		struct kmem_cache *slab;
		struct page *first_page;
	};
};

#endif
