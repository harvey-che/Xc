#include <Xc/mm.h>
#include <asm/e820.h>
#include <Xc/kernel.h>
#include <asm/bootparam.h>

u32 max_low_pfn_mapped;
u32 max_pfn_mapped;

struct boot_params boot_params;

extern char __brk_base[];
u32 _brk_start = (u32)__brk_base;
u32 _brk_end = (u32)__brk_base;

void start_kernel()
{

	early_mem_init();

	memblock_init();
    sanitize_e820_map(e820.map, ARRAY_SIZE(e820.map), &e820.nr_map);
    memblock_x86_fill();

    /* Note! the direct mapping page tables are setup up before bootmem is initialized, it gets pages
	 * from low physical memory
	 */

    setup_paging();

	initmem_init();

	kmem_cache_init();
}
