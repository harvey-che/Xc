#include <Xc/mm.h>

u32 max_low_pfn_mapped;
u32 max_pfn_mapped;

u32 boot_params[128];

extern char __brk_base[];
u32 _brk_start = (u32)__brk_base;
u32 _brk_end = (u32)__brk_base;

void start_kernel()
{
	early_mem_init();
    setup_paging();
}
