#include <Xc/mm.h>
#include <asm/e820.h>
#include <Xc/kernel.h>
#include <asm/bootparam.h>
#include <Xc/mmzone.h>
#include <Xc/slab.h>
#include <Xc/bootmem.h>
#include <Xc/memblock.h>
#include <asm/sections.h>

unsigned long max_low_pfn_mapped;
unsigned long max_pfn_mapped;

struct boot_params boot_params;

unsigned long _brk_start = (unsigned long)__brk_base;
unsigned long _brk_end = (unsigned long)__brk_base;
void *kmem_list[128];

static void reserve_brk(void)
{
    if (_brk_end > _brk_start)
		memblock_x86_reserve_range(__pa(_brk_start), __pa(_brk_end), "BRK");

	_brk_start = 0;
}

static void mm_init(void)
{
    mem_init();
	kmem_cache_init();
}

static inline void my_sti()
{
    asm volatile("sti\n\t"
			     "jmp 1f\n\t"
				 "1: \n\t"
				 "jmp 2f\n\t"
				 "2:"
			     : : :"memory");
}

static inline void my_cli()
{
    asm volatile("cli": : :"memory");
}

void start_kernel()
{
	int i = 0;
	unsigned long flags;


	early_print_str("Starting up...\n");

	local_irq_disable();

	/* i386_start_kernel()->memblock_init(); */
	memblock_init();
	memblock_x86_reserve_range(__pa(&_text), __pa(&__bss_stop), "TEXT DATA BSS");

	memcpy(&e820.map, &boot_params.e820_map, sizeof(e820.map));
	e820.nr_map = boot_params.e820_entries;
    sanitize_e820_map(e820.map, ARRAY_SIZE(e820.map), &e820.nr_map);

	/* Enter setup_arch() */
	max_pfn = e820_end_of_ram_pfn();
    find_low_pfn_range();
	
	reserve_brk();
	memblock.current_limit = get_max_mapped();
    memblock_x86_fill();

    /* By harvey, Note!!! 
	 * the direct mapping page tables are setup up before bootmem is initialized, 
	 * it gets pages from low physical memory
	 */
    /* setup_arch()->init_memory_mapping(); */
    
	max_low_pfn_mapped = init_memory_mapping(0, max_low_pfn << PAGE_SHIFT);
	max_pfn_mapped = max_low_pfn_mapped;

	memblock.current_limit = get_max_mapped();

	/* setup_arch()->initmem_init(); */
	initmem_init();
    /*
	 * setup_arch()->paging_init()->pagetable_init();
	 *                            ->zone_sizes_init();
	 */
	zone_sizes_init();
    /* end -- setup_arch() */

	build_all_zonelists(NULL);

	/* mm_init()->kmem_cache_init(); */
    mm_init();

	local_irq_enable();

	gfp_allowed_mask = __GFP_BITS_MASK;
	kmem_cache_late();
	/* Test Routine */
    memset(kmem_list, 0, sizeof(kmem_list)); 
    kmem_list[0] = kmalloc(90, GFP_KERNEL);
	kmem_list[1] = kmalloc(190, GFP_KERNEL);
}
