#include <Xc/init.h>
#include <Xc/mm.h>
#include <asm/e820.h>
#include <Xc/kernel.h>
#include <asm/bootparam.h>
#include <Xc/mmzone.h>
#include <Xc/slab.h>
#include <Xc/bootmem.h>
#include <Xc/memblock.h>
#include <asm/sections.h>
#include <Xc/jiffies.h>
#include <Xc/delay.h>
#include <Xc/interrupt.h>
#include <Xc/sched.h>

unsigned long max_low_pfn_mapped;
unsigned long max_pfn_mapped;

struct boot_params boot_params;

bool early_boot_irqs_disabled;

unsigned long _brk_start = (unsigned long)__brk_base;
unsigned long _brk_end = (unsigned long)__brk_base;

char volatile ticktick = 'c';
extern char *tick2;

void (* late_time_init)(void);

extern void init_IRQ(void);

extern void time_init(void);

/* By harvey, for test */
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

#define LPS_PREC 8
static unsigned long __cpuinit calibrate_delay_converge(void)
{
    unsigned long lpj, lpj_base, ticks, loopadd, loopadd_base, chop_limit;
	int trials = 0, band = 0, trial_in_band = 0;

	lpj = (1 << 12);

	ticks = jiffies;
	while (ticks == jiffies)
		;
	ticks = jiffies;
	do {
        if (++trial_in_band == (1 << band)) {
            ++band;
			trial_in_band = 0;
		}
		__delay(lpj * band);
		trials += band;
	} while (ticks == jiffies);

	trials -= band;
	loopadd_base = lpj * band;
	lpj_base = lpj * trials;

recalibrate:
	lpj = lpj_base;
	loopadd = loopadd_base;

	chop_limit = lpj >> LPS_PREC;
	while (loopadd > chop_limit) {
        lpj += loopadd;
		ticks = jiffies;
		while (ticks == jiffies)
			;
		ticks = jiffies;
		__delay(lpj);
		if (jiffies != ticks)
			lpj -= loopadd;
		loopadd >>= 1;
	}

	if (lpj + loopadd * 2 == lpj_base + loopadd_base * 2) {
        lpj_base = lpj;
		loopadd_base <<= 2;
		goto recalibrate;
	}

	loops_per_jiffy = lpj;

	return lpj;
}
/* kernel/sched_clock.c */
int sched_clock_running;
void sched_clock_init(void)
{
    sched_clock_running = 1;
}
/* end -- kernel/sched_clock.c */

void __init __weak thread_info_cache_init(void)
{
}

static void rest_init(void)
{
    int pid;

	kernel_thread(kernel_init, NULL, CLONE_FS | CLONE_SIGHAND);

	pid = kernel_thread(kthread, NULL, CLONE_FS | CLONE_FILES);

    /* kthread_task = find_task_by_pid_ns(pid, &init_pid_ns); */

	init_idle_bootup_task(current);
	preempt_enable_no_resched();
	schedule();

	preempt_disable();
	cpu_idle();
}

void start_kernel()
{
	int i = 0;
	unsigned long flags;


	early_print_str("Starting up...\n");

	local_irq_disable();
	early_boot_irqs_disabled = true;


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

    trap_init();
	/* mm_init()->kmem_cache_init(); */
    mm_init();

	early_irq_init();
	init_IRQ();

	time_init();


	early_boot_irqs_disabled = false;
	local_irq_enable();


	gfp_allowed_mask = __GFP_BITS_MASK;
	kmem_cache_init_late();

	if (late_time_init)
		late_time_init();

	sched_clock_init();
	calibrate_delay_converge();
    /* pidmap_init(); */

	thread_info_cache_init();
	
	fork_init(totalram_pages);
	while (1) {
        int debug = 0;
		debug++;
	}

	/* Test Routine */
    memset(kmem_list, 0, sizeof(kmem_list)); 
    kmem_list[0] = kmalloc(90, GFP_KERNEL);
	kmem_list[1] = kmalloc(190, GFP_KERNEL);
}
