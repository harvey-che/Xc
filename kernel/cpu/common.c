#include <Xc/init.h>
#include <asm/desc.h>
#include <Xc/atomic.h>
#include <Xc/smp.h>
#include <Xc/sched.h>
#include <Xc/mm_types.h>

DEFINE_PER_CPU_PAGE_ALIGNED(struct gdt_page, gdt_page) = { .gdt = {
    [GDT_ENTRY_KERNEL_CS]       = GDT_ENTRY_INIT(0xc09a, 0, 0xfffff),
	[GDT_ENTRY_KERNEL_DS]       = GDT_ENTRY_INIT(0xc092, 0, 0xfffff),
	[GDT_ENTRY_DEFAULT_USER_CS] = GDT_ENTRY_INIT(0xc0fa, 0, 0xfffff),
	[GDT_ENTRY_DEFAULT_USER_DS] = GDT_ENTRY_INIT(0xc0f2, 0, 0xfffff),

	[GDT_ENTRY_PNPBIOS_CS32]    = GDT_ENTRY_INIT(0x409a, 0, 0xffff),
	[GDT_ENTRY_PNPBIOS_CS16]    = GDT_ENTRY_INIT(0x009a, 0, 0xffff),
	[GDT_ENTRY_PNPBIOS_DS]      = GDT_ENTRY_INIT(0x0092, 0, 0xffff),
	[GDT_ENTRY_PNPBIOS_TS1]     = GDT_ENTRY_INIT(0x0092, 0, 0),
	[GDT_ENTRY_PNPBIOS_TS2]     = GDT_ENTRY_INIT(0x0092, 0, 0),

	[GDT_ENTRY_APMBIOS_BASE]       = GDT_ENTRY_INIT(0x409a, 0, 0xffff),
	[GDT_ENTRY_APMBIOS_BASE + 1]   = GDT_ENTRY_INIT(0x009a, 0, 0xffff),
	[GDT_ENTRY_APMBIOS_BASE + 2]   = GDT_ENTRY_INIT(0x4092, 0, 0xffff),
	[GDT_ENTRY_ESPFIX_SS]          = GDT_ENTRY_INIT(0xc092, 0, 0xfffff),
	[GDT_ENTRY_PERCPU]             = GDT_ENTRY_INIT(0xc092, 0, 0xfffff),
	GDT_STACK_CANARY_INIT
} };


void __cpuinit cpu_init(void)
{
	int cpu = smp_processor_id();
	struct task_struct *curr = current;
	struct tss_struct *t = &per_cpu(init_tss, cpu);
	struct thread_struct *thread = &curr->thread;

    load_idt((const struct desc_ptr *)&idt_descr);
	/* switch_to_new_gdt(cpu); */
    
	atomic_inc(&init_mm.mm_count);
	curr->active_mm = &init_mm;
	/* BUG_ON(curr->mm); */
	/* enter_lazy_tlb(&init_mm, curr); */

	load_sp0(t, thread);
	set_tss_desc(cpu, t);
	load_TR_desc();
	load_LDT(&init_mm.context);

	t->x86_tss.io_bitmap_base = offsetof(struct tss_struct, io_bitmap);

	/* clear_all_debug_regs(); */
	/* dbg_restore_debug_regs(); */

	/* fpu_init(); */
	/* xsave_init(); */
}
