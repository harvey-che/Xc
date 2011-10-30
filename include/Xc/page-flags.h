#ifndef _XC_PAGE_FLAGS_H
#define _XC_PAGE_FLAGS_H

#include <Xc/mm_types.h>

enum pageflags {
	PG_locked,		/* Page is locked. Don't touch. */
	PG_error,
	PG_referenced,
	PG_uptodate,
	PG_dirty,
	PG_lru,
	PG_active,
	PG_slab,
	PG_owner_priv_1,	/* Owner use. If pagecache, fs may use*/
	PG_arch_1,
	PG_reserved,
	PG_private,		/* If pagecache, has fs-private data */
	PG_private_2,		/* If pagecache, has fs aux data */
	PG_writeback,		/* Page is under writeback */
/* #ifdef CONFIG_PAGEFLAGS_EXTENDED */
	PG_head,		/* A head page */
	PG_tail,		/* A tail page */
/* #else 
	PG_compound,		 A compound page
 #endif */
	PG_swapcache,		/* Swap page: swp_entry_t in private */
	PG_mappedtodisk,	/* Has blocks allocated on-disk */
	PG_reclaim,		/* To be reclaimed asap */
	PG_swapbacked,		/* Page is backed by RAM/swap */
	PG_unevictable,		/* Page is "unevictable"  */
/* #ifdef CONFIG_MMU */
	PG_mlocked,		/* Page is vma mlocked */
/* #endif */
#ifdef CONFIG_ARCH_USES_PG_UNCACHED
	PG_uncached,		/* Page has been mapped as uncached */
#endif
#ifdef CONFIG_MEMORY_FAILURE
	PG_hwpoison,		/* hardware poisoned page. Don't touch */
#endif
#ifdef CONFIG_TRANSPARENT_HUGEPAGE
	PG_compound_lock,
#endif
	__NR_PAGEFLAGS,

	/* Filesystems */
	PG_checked = PG_owner_priv_1,

	/* Two page bits are conscripted by FS-Cache to maintain local caching
	 * state.  These bits are set on pages belonging to the netfs's inodes
	 * when those inodes are being locally cached.
	 */
	PG_fscache = PG_private_2,	/* page backed by cache */

	/* XEN */
	PG_pinned = PG_owner_priv_1,
	PG_savepinned = PG_dirty,

	/* SLOB */
	PG_slob_free = PG_private,

	/* SLUB */
	PG_slub_frozen = PG_active,
};

/* make prepare generates the bounds.h which includes this marco */
#define NR_PAGEFLAGS __NR_PAGEFLAGS

#define TESTPAGEFLAG(uname, lname)    \
	static inline int Page##uname(const struct page *page)    \
    { return test_bit(PG_##lname, &page->flags); }

#define SETPAGEFLAG(uname, lname)     \
	static inline void SetPage##uname(struct page *page)    \
    { set_bit(PG_##lname, &page->flags); }

#define CLEARPAGEFLAG(uname, lname)    \
	static inline void ClearPage##uname(struct page *page)    \
    { clear_bit(PG_##lname, &page->flags); }

#define __SETPAGEFLAG(uname, lname)    \
	static inline void __SetPage##uname(struct page *page)    \
    { __set_bit(PG_##lname, &page->flags); }

#define __CLEARPAGEFLAG(uname, lname)    \
	static inline void __ClearPage##uname(struct page *page)  \
    { __clear_bit(PG_##lname, &page->flags); }

#define PAGEFLAG(uname, lname) TESTPAGEFLAG(uname, lname)    \
	SETPAGEFLAG(uname, lname) CLEARPAGEFLAG(uname, lname)

#define __PAGEFLAG(uname, lname) TESTPAGEFLAG(uname, lname)    \
	__SETPAGEFLAG(uname, lname) __CLEARPAGEFLAG(uname, lname)

#define PAGEFLAG_FALSE(uname)    \
	static inline int Page##uname(const struct page *page) { return 0; }

#define __PG_MLOCKED (1 << PG_mlocked)
#define __PG_COMPOUND_LOCK 0


#define PAGE_FLAGS_CHECK_AT_FREE \
	(1 << PG_lru	 | 1 << PG_locked    | \
	 1 << PG_private | 1 << PG_private_2 | \
	 1 << PG_writeback | 1 << PG_reserved | \
	 1 << PG_slab	 | 1 << PG_swapcache | 1 << PG_active | \
	 1 << PG_unevictable | __PG_MLOCKED | __PG_HWPOISON | \
	 __PG_COMPOUND_LOCK)

#define PAGE_FLAGS_CHECK_AT_PREP	((1 << NR_PAGEFLAGS) - 1)

static inline int PageCompound(struct page *page)
{
    return page->flags & ((1L << PG_head) | (1L << PG_tail));
}

__PAGEFLAG(Head, head)

CLEARPAGEFLAG(Head, head)

__PAGEFLAG(Tail, tail)
__PAGEFLAG(Slab, slab)
/*
PAGEFLAG(HWPoison, hwpoison)
TESTSCFLAG(HWpoison, hwposion)
#define __PG_HWPOISON (1UL << PG_hwpoison)
*/
PAGEFLAG_FALSE(HWPoison)
#define __PG_HWPOISON 0

unsigned long get_pageblock_flags_group(struct page *page, int start_bitidx, int end_idx);


#endif
