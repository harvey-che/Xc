#ifndef _ASM_X86_PROCESSOR_H
#define _ASM_X86_PROCESSOR_H

#include <asm/pagetable.h>

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

#endif
