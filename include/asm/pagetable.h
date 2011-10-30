#ifndef _ASM_X86_PGTABLE_32_H
#define _ASM_X86_PGTABLE_32_H

#include <asm/page.h>
#include <asm/pgtable_types.h>
#include <Xc/types.h>

#ifndef __ASSEMBLY__

typedef struct {
    u32 pgd;
} pgd_t;

typedef struct {
    u32 pud;
} pud_t;

typedef struct {
    u32 pmd;
} pmd_t;

typedef struct {
    u32 pte;
} pte_t;

typedef struct {
    u32 pgprot;
} pgprot_t;

#define __pgprot(x) ((pgprot_t){(x)})

#define __pgd(x) ((pgd_t){(x)})
#define __pud(x) ((pud_t){(x)})
#define __pmd(x) ((pmd_t){(x)})
#define __pte(x) ((pte_t){(x)})

extern pgd_t swapper_pg_dir[1024];


#define pgd_index(address) (((u32)(address) >> PGDIR_SHIFT) & (PTRS_PER_PGD - 1))

#define pud_index(address) (((u32)(address) >> PUD_SHIFT) & (PTRS_PER_PUD - 1))

static inline pud_t* pud_offset(pgd_t *pgd, u32 address)
{
    return (pud_t*)pgd;
}

#define pmd_index(address) (((u32)(address) >> PMD_SHIFT) & (PTRS_PER_PMD - 1))

static inline pmd_t* pmd_offset(pud_t *pud, u32 address)
{
    return (pmd_t*)pud;
}

#define pte_index(address) (((u32)(address) >> PAGE_SHIFT) & (PTRS_PER_PTE - 1))

static inline pte_t* pte_offset(pmd_t *pmd, u32 address)
{
    return (pte_t*) __va(pmd->pmd & PTE_PFN_MASK) + pte_index(address);
}

static inline void set_pgd(pgd_t *pgdp, pgd_t pgd)
{
    *pgdp = pgd;
}

static inline void set_pmd(pmd_t *pmdp, pmd_t pmd)
{
    *pmdp = pmd;
}

static inline void set_pte(pte_t *ptep, pte_t pte)
{
    *ptep = pte;
}


static inline pte_t pfn_pte(u32 pfn, u32  pgprot)
{
    return __pte(pfn << PAGE_SHIFT | pgprot);
}

#endif

#endif
