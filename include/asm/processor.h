#ifndef _ASM_X86_PROCESSOR_H
#define _ASM_X86_PROCESSOR_H

#include <asm/pagetable.h>

#ifndef __ASSEMBLY__

static unsigned long __force_order;

static inline unsigned long read_cr3(void)
{
    unsigned long val;
	asm volatile ("mov %%cr3, %0\n\t": "=r"(val), "=m"(__force_order));
	return val;
}

static inline void write_cr3(unsigned long val)
{
    asm volatile ("mov %0, %%cr3": :"r"(val), "m"(__force_order));
}

static inline void __flush_tlb(void)
{
   write_cr3(read_cr3());
}

static inline void __flush_tlb_all(void)
{
    __flush_tlb();
}

static inline void load_cr3(pgd_t *pgdir)
{
    write_cr3(__pa(pgdir));
}

static inline void rep_nop(void)
{
    asm volatile("rep; nop" ::: "memory");
}

static inline void cpu_relax(void)
{
    rep_nop();
}

typedef struct {
    unsigned long seg;
} mm_segment_t;

#ifdef CONFIG_X86_32

#define THREAD_SIZE_LONGS (THREAD_SIZE / sizeof(unsigned long))
#define KSTK_TOP(info)    \
	({                    \
	     unsigned long *__ptr = (unsigned long *)(info);    \
	     (unsigned long)(&__ptr[THREAD_SIZE_LONGS]); })

#define task_pt_regs(task)    \
	({                        \
	     struct pt_regs *__regs__    \
	     __regs__ = (struct pt_regs *)(KSTK_TOP(task_stack_page(task)) - 8);    \
	     __regs - 1; })

#else

#endif /* CONFIG_X86_32 */

#endif /* __ASSEMBLY__ */

#endif
