#include <asm/pagetable.h>
#include <stddef.h>
#include <Xc/kernel.h>
#include <Xc/string.h>
#include <asm/processor.h>
#include <Xc/bootmem.h>
#include <Xc/mm.h>
#include <asm/dma.h>
#include <Xc/mmzone.h>
#include <asm/sections.h>
#include <Xc/memblock.h>
#include <Xc/swap.h>
#include <asm/io.h>

unsigned long pgt_buf_start = 0;
unsigned long pgt_buf_end = 0;
unsigned long pgt_buf_top = 0;

int after_bootmem = 0;

static unsigned int highmem_pages = -1;

struct map_range {
	unsigned long start;
	unsigned long end;
	unsigned page_size_mask;
};

#define NR_RANGE_MR 3

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

/*
void early_mem_init()
{
	u32 tables_size;
	pgt_buf_start = (((u32)_end - PAGE_OFFSET) + PAGE_SIZE - 1) >> PAGE_SHIFT;
	pgt_buf_end = pgt_buf_start;

	//ptes = ((boot_params[0] + PAGE_SIZE - 1) >> PAGE_SHIFT);
	tables_size = roundup(max_pfn * sizeof (pte_t), PAGE_SIZE);
    pgt_buf_top = pgt_buf_start + (tables_size >> PAGE_SHIFT);
}
*/

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
        if (after_bootmem) {
            page_table = (pte_t*)alloc_bootmem_pages(PAGE_SIZE);
		}
		else
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

	end_pfn = max_pfn;
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


void setup_bootmem_allocator(void)
{
    after_bootmem = 1;
}

void initmem_init(void)
{
    memblock_x86_register_active_regions(0, 0, max_low_pfn);
	setup_bootmem_allocator();
}

void zone_sizes_init(void)
{
    unsigned long max_zone_pfns[MAX_NR_ZONES];

	memset(max_zone_pfns, 0, sizeof(max_zone_pfns));
    max_zone_pfns[ZONE_DMA] = virt_to_phys((char*)MAX_DMA_ADDRESS) >> PAGE_SHIFT;
	max_zone_pfns[ZONE_NORMAL] = max_low_pfn;
	free_area_init_nodes(max_zone_pfns);
}

void lowmem_pfn_init(void)
{
	/* CONFIG_HIGHMEM is not defined */
    max_low_pfn = max_pfn;
	if (highmem_pages == -1)
		highmem_pages = 0;
}

void highmem_pfn_init(void)
{
    max_low_pfn = MAXMEM_PFN;
	if (highmem_pages == -1)
		highmem_pages = max_pfn - MAXMEM_PFN;
	if (highmem_pages + MAXMEM_PFN < max_pfn)
		max_pfn = MAXMEM_PFN + highmem_pages;
	if (highmem_pages + MAXMEM_PFN > max_pfn) {
        highmem_pages = 0;
	}
	max_pfn = MAXMEM_PFN;
}

void find_low_pfn_range(void)
{
    if (max_pfn <= MAXMEM_PFN)
	    lowmem_pfn_init();
    else
	    highmem_pfn_init();
}

static int save_mr(struct map_range *mr, int nr_range, unsigned long start_pfn, 
		           unsigned long end_pfn, unsigned long page_size_mask)
{
    if (start_pfn < end_pfn) {
        if (nr_range >= NR_RANGE_MR)
			panic("run out of range for init_memory_mapping\n");
		mr[nr_range].start = start_pfn << PAGE_SHIFT;
		mr[nr_range].end = end_pfn << PAGE_SHIFT;
		mr[nr_range].page_size_mask = page_size_mask;
		nr_range++;
	}
	return nr_range;
}

static void find_early_table_space(unsigned long end, int use_pse, int use_gbpages)
{
    unsigned long puds, pmds, ptes, tables, start = 0, good_end = end;
	phys_addr_t base;

	puds = (end + PUD_SIZE - 1) >> PUD_SHIFT;
	tables = roundup(puds * sizeof(pud_t), PAGE_SIZE);

	if (use_gbpages) {
        unsigned long extra;
		extra = end - ((end >> PUD_SHIFT) << PUD_SHIFT);
		pmds = (extra + PMD_SIZE - 1) >> PMD_SHIFT;
	} else
		pmds = (end + PMD_SIZE - 1) >> PMD_SHIFT;

	tables += roundup(pmds * sizeof(pmd_t), PAGE_SIZE);

	if (use_pse) {
        unsigned long extra;
		extra = end - ((end >> PMD_SHIFT) << PMD_SHIFT);
		extra += PMD_SIZE;
		ptes = (extra + PAGE_SIZE - 1) >> PAGE_SHIFT;
	} else
		ptes = (end + PAGE_SIZE - 1) >> PAGE_SHIFT;

	tables += roundup(ptes * sizeof(pte_t), PAGE_SIZE);
	tables += roundup(__end_of_fixed_addresses * sizeof(pte_t), PAGE_SIZE);

	good_end = max_pfn_mapped << PAGE_SHIFT;
	base = memblock_find_in_range(start, good_end, tables, PAGE_SIZE);
	if (base == MEMBLOCK_ERROR)
		panic("Cannot find space for the kernel page tables");
	
	pgt_buf_start = base >> PAGE_SHIFT;
	pgt_buf_end = pgt_buf_start;
	pgt_buf_top = pgt_buf_start + (tables >> PAGE_SHIFT);
}

static inline int is_kernel_text(unsigned long addr)
{
    if (addr >= (unsigned long)_text && addr <= (unsigned long)_edata)
		return 1;
	return 0;
}

unsigned long kernel_physical_mapping_init(unsigned long start, unsigned long end,
		                                   unsigned long page_size_mask)
{
    int use_pse = page_size_mask == (1 << PG_LEVEL_2M);
	unsigned long last_map_addr = end;
	unsigned long start_pfn, end_pfn;
	pgd_t *pgd_base = swapper_pg_dir;
	int pgd_idx, pmd_idx, pte_ofs;
	unsigned long pfn;
	pgd_t *pgd;
	pmd_t *pmd;
	pte_t *pte;
	unsigned pages_2m, pages_4k;
	int mapping_iter;

	start_pfn = start >> PAGE_SHIFT;
	end_pfn = end >> PAGE_SHIFT;

	mapping_iter = 1;
	use_pse = 0;

repeat:
	pages_2m = pages_4k = 0;
	pfn = start_pfn;

    pgd_idx = pgd_index((pfn << PAGE_SHIFT) + PAGE_OFFSET);
    pgd = pgd_base + pgd_idx;
	for (; pgd_idx < PTRS_PER_PGD; pgd++, pgd_idx++) {
        pmd = one_md_table_init(pgd);
		if (pfn >= end_pfn)
			continue;

		pmd_idx = 0;

		for (; pmd_idx < PTRS_PER_PMD && pfn < end_pfn; pmd++, pmd_idx++) {
			unsigned long addr = pfn * PAGE_SIZE + PAGE_OFFSET;

			pte = one_page_table_init(pmd);
			pte_ofs = pte_index((pfn << PAGE_SHIFT) + PAGE_OFFSET);
			pte += pte_ofs;

			for (; pte_ofs < PTRS_PER_PTE && pfn < end_pfn; 
				  pte++, pfn++, pte_ofs++, addr += PAGE_SIZE) {
				unsigned long prot = __PAGE_KERNEL;
				unsigned long init_prot = PTE_IDENT_ATTR;

				if (is_kernel_text(addr))
					prot = __PAGE_KERNEL_EXEC;

				pages_4k++;
				if (mapping_iter == 1) {
					set_pte(pte, pfn_pte(pfn, init_prot));
					last_map_addr = (pfn << PAGE_SHIFT) + PAGE_SIZE;
				} else
				    set_pte(pte, pfn_pte(pfn, prot));
			}
		}
	}
    
	if (mapping_iter == 1) {
		/* By harvey, unmapping the lower linear address range at 0 */
        set_pmd((pmd_t *)pgd_base, __pmd(0));


	    load_cr3(swapper_pg_dir);
        __flush_tlb_all();
		mapping_iter = 2;

		goto repeat;
	}
    return last_map_addr;
}

void native_pagetable_reserve(u64 start, u64 end)
{
    memblock_x86_reserve_range(start, end, "PGTABLE");
}

unsigned long init_memory_mapping(unsigned long start, unsigned long end)
{
    unsigned long page_size_mask = 0;
	unsigned long start_pfn, end_pfn;
	unsigned long ret = 0;
	unsigned long pos;

	struct map_range mr[NR_RANGE_MR];
	int nr_range, i;
	
	memset(mr, 0, sizeof(mr));
	nr_range = 0;

	start_pfn = start >> PAGE_SHIFT;
	pos = start_pfn << PAGE_SHIFT;

	if (pos == 0)
		end_pfn = 1 << (PMD_SHIFT - PAGE_SHIFT);
	else
		end_pfn = ((pos + (PMD_SIZE - 1)) >> PMD_SHIFT) << (PMD_SHIFT - PAGE_SHIFT);

	if (end_pfn > (end >> PAGE_SHIFT))
		end_pfn = end >> PAGE_SHIFT;
	if (start_pfn < end_pfn) {
        nr_range = save_mr(mr, nr_range, start_pfn, end_pfn, 0);
		pos = end_pfn << PAGE_SHIFT;
	}

	start_pfn = ((pos + (PMD_SIZE - 1)) >> PMD_SHIFT) << (PMD_SHIFT - PAGE_SHIFT);
	end_pfn = (end >> PMD_SHIFT) << (PMD_SHIFT - PAGE_SHIFT);

	if (start_pfn < end_pfn) {
        nr_range = save_mr(mr, nr_range, start_pfn, end_pfn, 
				           page_size_mask & (1 << PG_LEVEL_2M));
		pos = end_pfn << PAGE_SHIFT;
	}

	start_pfn = pos >> PAGE_SHIFT;
	end_pfn = end >> PAGE_SHIFT;
	nr_range = save_mr(mr, nr_range, start_pfn, end_pfn, 0);

	for (i = 0; nr_range > 1 && i < nr_range - 1; i++) {
        unsigned long old_start;
		if (mr[i].end != mr[i + 1].start || mr[i].page_size_mask != mr[i + 1].page_size_mask)
			continue;
		old_start = mr[i].start;
		memmove(&mr[i], &mr[i + 1], (nr_range - 1 - i) * sizeof(struct map_range));
		mr[i--].start = old_start;
		nr_range--;
	}

	if (!after_bootmem)
		find_early_table_space(end, 0 /*use_pse */, 0 /* use_gdpages */);
	for (i = 0; i < nr_range; i++)
		ret = kernel_physical_mapping_init(mr[i].start, mr[i].end, mr[i].page_size_mask);
	/* early_ioremap_page_table_range_init() */
	load_cr3(swapper_pg_dir);
	__flush_tlb_all();

	if (!after_bootmem && pgt_buf_end > pgt_buf_start)
        native_pagetable_reserve(PFN_PHYS(pgt_buf_start), PFN_PHYS(pgt_buf_end));

	/*
	 * if (!after_bootmem)
	 *	early_memtest(start, end);
	 */

	return ret >> PAGE_SHIFT;
}

void mem_init(void)
{
    /* BUG_ON(!mem_map); */
	totalram_pages += free_all_bootmem();
}
