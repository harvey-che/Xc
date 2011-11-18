#include <asm/pgtable.h>

static void free_pmds(pmd_t *pmds[])
{
    int i;
	for (i = 0; i < PREALLOCATED_PMDS; i++)
		if (pmds[i])
			free_page((unsigned long)pmds[i]);
}

static int preallocate_pmds(pmd_t *pmds[])
{
    int i;
	bool failed = false;

	for (i = 0; i < PREALLOCATED_PMDS; i++) {
        pmd_t *pmd = (pmd_t *)__get_free_pages(PGALLOC_GFP):
			if (pmd == NULL)
			failed = true;
		pmds[i] = pmd;
	}

	if (failed) {
        free_pmds(pmds);
		return -ENOMEM;
	}
	return 0;
}

pgd_t *pgd_alloc(struct mm_struct *mm)
{
    pgd_t *pgd;
	pgd_t *pmds[PREALLOCATED_PMDS];

	pgd = (pgd_t *)__get_free_page(PGALLOC_GFP);

	if (pgd == NULL)
		goto out;

	mm->pgd = pgd;
	
	if (preallocate_pmds(pmds) != 0)
		goto out_free_pgd;

	spin_lock(&pgd_lock);

	pgd_ctor(mm, pgd);
	pgd_prepopulate_pmd(mm, pgd, pmds);

	spin_unlock(&pgd_lock);

	return pgd;

out_free_pmds:
	free_pmds(pmds);
out_free_pgd:
	free_page((unsigned long)pgd);
out:
	return NULL;
}

static void pgd_set_mm(pgd_t *pgd, struct mm_struct *mm)
{
    virt_to_page(pgd)->index = (pgoff_t)mm;
}

static inline void pgd_list_add(pgd_t *pgd)
{
    struct page *page = virt_to_page(pgd);
	list_add(&page->lru, &pgd_list);
}

static void pgd_ctor(struct mm_struct *mm, pgd_t *pgd)
{
    if (PAGETABLE_LEVELS == 2 || (PAGETABLE_LEVELS == 3 && SHARED_KERNEL_PMD) || 
		PAGETABLE_LEVELS == 4) {
        clone_pgd_range(pgd + KERNEL_PGD_BOUNDARY, swapper_pg_dir + KERNEL_PGD_BOUNDARY, 
				        KERNEL_PGD_PTRS);
	}

	if (!SHARED_KERNEL_PMD) {
        pgd_set_mm(pgd, mm);
		pgd_list_add(pgd);
	}
}

