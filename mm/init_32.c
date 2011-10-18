#include <asm/pagetable.h>
#include <stddef.h>
#include <Xc/kernel.h>
#include <string.h>
#include <asm/processor.h>

#include <Xc/mm.h>

extern u8 _end[0];

u32 pgt_buf_start = 0;
u32 pgt_buf_end = 0;
u32 pgt_buf_top = 0;
static void* alloc_low_page()
{
	u32 pfn;
	void * addr;
    if (pgt_buf_end >= pgt_buf_top)
	{
        panic("alloc_low_page: run out of memory");
	}
    
	pfn = pgt_buf_end++;
    addr = __va(pfn << PAGE_SHIFT);
	memset(addr, 0, PAGE_SIZE);
    return addr;
}


void early_mem_init()
{
	u32 ptes, tables_size;
	pgt_buf_start = (((u32)_end - PAGE_OFFSET) + PAGE_SIZE - 1) >> PAGE_SHIFT;
	pgt_buf_end = pgt_buf_start;

	ptes = ((boot_params[0] + PAGE_SIZE - 1) >> PAGE_SHIFT);
	tables_size = roundup(ptes * sizeof (pte_t), PAGE_SIZE);
    pgt_buf_top = pgt_buf_start + (tables_size >> PAGE_SHIFT);
}

static pmd_t* one_md_table_init(pgd_t *pgd)
{
    pud_t *pud;
	pmd_t *pmd_table;

	pud = pud_offset(pgd, 0);
	pmd_table = pmd_offset(pud, 0);
	
	return pmd_table;
}

static pte_t* one_page_table_init(pmd_t *pmd)
{
    if (!(pmd->pmd & _PAGE_PRESENT)) {
        pte_t *page_table = NULL;

		page_table = (pte_t*)alloc_low_page();
		set_pmd(pmd, __pmd(__pa(page_table) | _PAGE_TABLE));
	}
	return pte_offset(pmd, 0);
}

void setup_paging(void)
{
	int pgd_idx, pmd_idx, pte_idx;
	u32 pfn = 0, end_pfn = 0;
	pgd_t *pgd;
	pmd_t *pmd;
	pte_t *pte;

	end_pfn = boot_params[0] >> PAGE_SHIFT;
    pgd_idx = pgd_index((pfn << PAGE_SHIFT) + PAGE_OFFSET);
    pgd = swapper_pg_dir + pgd_idx;
	for (; pgd_idx < PTRS_PER_PGD; pgd++, pgd_idx++) {
        pmd = one_md_table_init(pgd);
		pmd_idx = pmd_index(pfn << PAGE_SHIFT);

		for (; pmd_idx < PTRS_PER_PMD && pfn < end_pfn; pmd++, pmd_idx++) {
			pte = one_page_table_init(pmd);
			pte_idx = pte_index((pfn << PAGE_SHIFT) + PAGE_OFFSET);
			for (; pte_idx < PTRS_PER_PTE && pfn < end_pfn; pte++, pte_idx++, pfn++)
			{
				set_pte(pte, pfn_pte(pfn, __PAGE_KERNEL_EXEC));
			}
		}
	}

	load_cr3(swapper_pg_dir);

	__flush_tlb_all();
}


