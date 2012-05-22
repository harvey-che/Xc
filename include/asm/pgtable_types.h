#ifndef _ASM_X86_PGTABLE_DEFS_H
#define _ASM_X86_PGTABLE_DEFS_H

#include <asm/page_types.h>

/*
 *  asm/pgtable_types.h -> asm/pgtable_32_types.h -> asm/pgtable-2level_types.h
 *                      -> asm-generic/pgtable-nopmd.h
 *                      -> asm/page_types.h
 *
 */

#define PGDIR_SHIFT 22
#define PTRS_PER_PGD 1024

#define PUD_SHIFT 22
#define PTRS_PER_PUD 1
#define PUD_SIZE  (1UL << PUD_SHIFT)
#define PUD_MASK  (~(PUD_SIZE - 1))

#define PMD_SHIFT 22
#define PTRS_PER_PMD 1
#define PMD_SIZE (1UL << PMD_SHIFT)
#define PMD_MASK (~(PMD_SIZE - 1))

#define PAGE_SHIFT 12
#define PTRS_PER_PTE 1024

#define PTE_PFN_MASK 0xfffff000

#define HPAGE_SHIFT PMD_SHIFT
#define HUGETLB_PAGE_ORDER (HPAGE_SHIFT - PAGE_SHIFT)

#define _PAGE_PRESENT   0x00000001
#define _PAGE_RW        0x00000002
#define _PAGE_USER      0x00000004
#define _PAGE_ACCESSED  0x00000020
#define _PAGE_DIRTY     0x00000040
#define _PAGE_GLOBAL    0x00000100
#define _PAGE_NX        0x00000000

#define PTE_IDENT_ATTR 0x003    /* PRESENT + RW*/
#define PDE_IDENT_ATTR 0x067    /* PRESENT + RW +USER + DIRTY + ACCESSED */
#define PGD_IDENT_ATTR 0x001    /* PRESENT (no other attributes) */

#define _PAGE_TABLE (_PAGE_PRESENT | _PAGE_RW | _PAGE_USER |    \
                	 _PAGE_ACCESSED | _PAGE_DIRTY)

#define __PAGE_KERNEL_EXEC    (_PAGE_PRESENT | _PAGE_RW | _PAGE_DIRTY | _PAGE_ACCESSED |   \
		                       _PAGE_GLOBAL)

#define __PAGE_KERNEL  (__PAGE_KERNEL_EXEC | _PAGE_NX)

#ifndef __ASSEMBLY__

typedef struct {
    unsigned long pgd;
} pgd_t;

typedef struct {
    unsigned long pud;
} pud_t;

typedef struct {
    unsigned long pmd;
} pmd_t;

typedef struct {
    unsigned long pte;
} pte_t;

typedef struct {
    unsigned long pgprot;
} pgprot_t;

/* pgtable-nopmd.h */
struct mm_struct;
static inline void pmd_free(struct mm_struct *mm, pmd_t *pmd)
{
}
/* end -- pgtable-nopmd.h */

static inline pgd_t native_make_pgd(unsigned long val)
{
    return (pgd_t) { val };
}

/* variables defined in pgtable_32.c  */
#define __FIXADDR_TOP 0xfffff000
#define __VMALLOC_RESERVE  (128 << 20)
/* end -- pgtable_32.c */

/* fixmap.h */

#define FIXADDR_TOP ((unsigned long)__FIXADDR_TOP)
enum fixed_addresses {
	__end_of_permanent_fixed_addresses = 1024,
	__end_of_fixed_addresses = 1024,
};
#define FIXADDR_SIZE        (__end_of_permanent_fixed_addresses << PAGE_SHIFT)
#define FIXADDR_BOOT_SIZE   (__end_of_fixed_addresses << PAGE_SHIFT)
#define FIXADDR_START       (FIXADDR_TOP - FIXADDR_SIZE)
#define FIXADDR_BOOT_START  (FIXADDR_TOP - FIXADDR_BOOT_SIZE)
/* end -- fixmap.h */

/* pgtable_32_types.h */

#define LAST_PKMAP 1024
#define PKMAP_BASE ((FIXADDR_BOOT_START - PAGE_SIZE * (LAST_PKMAP + 1)) & PMD_MASK)
#define VMALLOC_END (FIXADDR_START - 2 * PAGE_SIZE)

#define MAXMEM (VMALLOC_END - PAGE_OFFSET - __VMALLOC_RESERVE)

/* end -- pgtable_32_types.h */

/* asm/setup.h */
#include <Xc/pfn.h>
#define MAXMEM_PFN PFN_DOWN(MAXMEM)
/* end -- asm/setup.h */

enum {
	PG_LEVEL_NONE,
	PG_LEVEL_4K,
    PG_LEVEL_2M,
	PG_LEVEL_1G,
	PG_LEVEL_NUM
};

#endif


#endif
