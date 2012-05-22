#ifndef _ASM_X86_PROCESSOR_H
#define _ASM_X86_PROCESSOR_H

#include <asm/pagetable.h>
#include <asm/cache.h>
#include <asm/desc_defs.h>
#include <Xc/percpu.h>
#include <asm/segment.h>

#define ARCH_MIN_TASKALIGN 16

#ifndef __ASSEMBLY__

struct task_struct;
struct mm_struct;

struct x86_hw_tss {
    unsigned short back_link, __blh;
	unsigned long  sp0;
	unsigned short ss0, __ss0h;
	unsigned long  sp1;

	unsigned short ss1, __ss1h;
	unsigned long  sp2;
	unsigned short ss2, __ss2h;
	unsigned long  __cr3;
	unsigned long  ip;
	unsigned long  flags;
	unsigned long  ax;
	unsigned long  cx;
	unsigned long  dx;
	unsigned long  bx;
	unsigned long  sp;
	unsigned long  bp;
	unsigned long  si;
	unsigned long  di;
	unsigned short es, __esh;
	unsigned short cs, __csh;
	unsigned short ss, __ssh;
	unsigned short ds, __dsh;
	unsigned short fs, __fsh;
	unsigned short gs, __gsh;
	unsigned short ldt, __ldt;
	unsigned short trace;
	unsigned short io_bitmap_base;
} __attribute__((packed));

DECLARE_PER_CPU_SHARED_ALIGNED(struct tss_struct, init_tss);

#define IO_BITMAP_BITS 65536
#define IO_BITMAP_BYTES (IO_BITMAP_BITS/8)
#define IO_BITMAP_LONGS (IO_BITMAP_BYTES/sizeof(long))
#define IO_BITMAP_OFFSET offsetof(struct tss_struct, io_bitmap)
#define INVALID_IO_BITMAP_OFFSET 0x8000

struct tss_struct{
    struct x86_hw_tss x86_tss;
	unsigned long io_bitmap[IO_BITMAP_LONGS + 1];
	unsigned long stack[64];
} ____cacheline_aligned;

struct thread_struct {
    struct desc_struct tls_array[GDT_ENTRY_TLS_ENTRIES];
	unsigned long      sp0;
	unsigned long      sp;

	unsigned long      sysenter_cs;

    unsigned long      ip;

	unsigned long      gs;
	
	/*
	struct perf_event  *ptrace_bps[HBP_NUM];
	unsigned long      debugreg6;
	unsigned long      ptrace_dr7;
    */
	/* Fault info */
	unsigned long      cr2;
	unsigned long      trap_no;
	unsigned long      error_code;

	/* struct fpu         fpu; */
    
	/* Virtual 86 mode info*/
	/*
	struct vm86_struct *vm86_info;
	unsigned long      screen_bitmap;
	unsigned long      v86flags;
	unsigned long      v86mask;
	unsigned long      saved_sp0;
	unsigned long      saved_fs;
	unsigned long      saved_gs;
    */
	unsigned long      *io_bitmap_ptr;
	unsigned long      iopl;
	unsigned           io_bitmap_max;
};

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

static inline void native_load_sp0(struct tss_struct *tss, struct thread_struct *thread)
{
    tss->x86_tss.sp0 = thread->sp0;
    /*
	if (tss->x86_tss.ss1 != thread->sysenter_cs) {
        tss->x86_tss.ss1 = thread->sysenter_cs;
		wrmsr(MSR_IA32_SYSENTER_CS, thread->sysenter_cs, 0);
	}
	*/
}

static inline void load_sp0(struct tss_struct *tss, struct thread_struct *thread)
{
    native_load_sp0(tss, thread);
}

typedef struct {
    unsigned long seg;
} mm_segment_t;

extern int kernel_thread(int (*fn)(void *), void *arg, unsigned long flags);
extern void prepare_to_copy(struct task_struct *tsk);

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

#define TASK_SIZE 0xc0000000

#define INIT_TSS {           \
	.x86_tss = {             \
		.sp0 = sizeof(init_stack) + (long)&init_stack, \
		.ss0 = __KERNEL_DS,                          \
		.ss1 = __KERNEL_CS,                          \
	    .io_bitmap_base = INVALIDE_IO_BITMAP_OFFSET, \
	},                                               \
	.io_bitmap = { [0 ... IO_BITMAP_LONGS] = ~0}, }

#define TASK_UNMAPPED_BASE (PAGE_ALIGN(TASK_SIZE / 3))

#else

#endif /* CONFIG_X86_32 */

#endif /* __ASSEMBLY__ */

#endif
