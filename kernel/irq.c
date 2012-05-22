#include <Xc/init.h>
#include <asm/ptrace.h>
#include <Xc/thread_info.h>
#include <Xc/gfp.h>
#include <Xc/percpu.h>
#include <Xc/hardirq.h>
#include <Xc/irq.h>
#include <Xc/irqdesc.h>
#include <asm/hw_irq.h>
#include <asm/apic.h>
#include <Xc/gfp.h>
#include <Xc/mm.h>
#include <asm/apic.h>

/* asm/uaccess.h */
#define MAKE_MM_SEG(s) ((mm_segment_t) { (s) })
/* end -- asm/uaccess.h */


/* asm/idle.h */
static inline void enter_idle(void) {}
static inline void exit_idle(void) {}
/* end -- asm/idle.h */

/* x86/kernel/irq_32.c */

union irq_ctx {
    struct thread_info tinfo;
	u32                stack[THREAD_SIZE / sizeof(u32)];
} __attribute__((aligned(THREAD_SIZE)));

static DEFINE_PER_CPU(union irq_ctx *, hardirq_ctx);
static DEFINE_PER_CPU(union irq_ctx *, softirq_ctx);

static void call_on_stack(void *func, void *stack)
{
    asm volatile ("xchgl %%ebx, %%esp \n"
			      "call *%%edi        \n"
				  "movl %%ebx, %%esp  \n"
				  : "=b" (stack)
				  : "0" (stack), "D" (func)
				  : "memory", "cc", "edx", "ecx", "edx");
}

DEFINE_PER_CPU(struct pt_regs *, irq_regs);

static inline int check_stack_overflow(void) { return 0; }
static inline void print_stack_overflow(void) {}

static inline int execute_on_irq_stack(int overflow, struct irq_desc *desc, int irq)
{
    union irq_ctx *curctx, *irqctx;
	u32 *isp, arg1, arg2;

	curctx = (union irq_ctx*)current_thread_info();
	irqctx = __this_cpu_read(hardirq_ctx);

	if (curctx == irqctx)
		return 0;

	isp = (u32 *)((char *)irqctx + sizeof(*irqctx));
	irqctx->tinfo.task = curctx->tinfo.task;
	irqctx->tinfo.previous_esp = current_stack_pointer;

	irqctx->tinfo.preempt_count = (irqctx->tinfo.preempt_count & ~SOFTIRQ_MASK) | 
		                          (curctx->tinfo.preempt_count & SOFTIRQ_MASK);

	if (overflow)
		call_on_stack(print_stack_overflow, isp);

	asm volatile( "xchgl %%ebx, %%esp \n"
			      "call *%%edi        \n"
				  "movl %%ebx, %%esp  \n"
				  : "=a" (arg1), "=d" (arg2), "=b" (isp)
				  : "0" (irq), "1" (desc), "2" (isp), "D" (desc->handle_irq)
				  : "memory", "cc", "ecx");
	return 1;
}

void __cpuinit irq_ctx_init(int cpu)
{
    union irq_ctx *irqctx;

	if (per_cpu(hardirq_ctx, cpu))
		return;

	irqctx = page_address(alloc_pages_node(cpu_to_node(cpu), THREAD_FLAGS, THREAD_ORDER));

	memset(&irqctx->tinfo, 0, sizeof(struct thread_info));
	irqctx->tinfo.cpu = cpu;
	irqctx->tinfo.preempt_count = HARDIRQ_OFFSET;
	irqctx->tinfo.addr_limit = MAKE_MM_SEG(0);

	per_cpu(hardirq_ctx, cpu) = irqctx;
	
	irqctx = page_address(alloc_pages_node(cpu_to_node(cpu), THREAD_FLAGS, THREAD_ORDER));
	memset(&irqctx->tinfo, 0, sizeof(struct thread_info));
	irqctx->tinfo.cpu = cpu;
	irqctx->tinfo.addr_limit = MAKE_MM_SEG(0);

	per_cpu(softirq_ctx, cpu) = irqctx;
}

bool handle_irq(unsigned irq, struct pt_regs *regs)
{
    struct irq_desc *desc;
	int overflow;

	overflow = check_stack_overflow();

	desc = irq_to_desc(irq);
    if (!desc)
		return false;

	if (!execute_on_irq_stack(overflow, desc, irq)) {
        if (overflow)
			print_stack_overflow();
		desc->handle_irq(irq, desc);
	}
	
	return true;
}

/* end -- x86/kernel/irq_32.c */

atomic_t irq_err_count;

void ack_bad_irq(unsigned int irq)
{
	/*
    if (printk_ratelimit()) {
	}
    */
	ack_APIC_irq();
}

static int counter_in_doIRQ = 0;
static char buf_in_doIRQ[16];
#include <Xc/kernel.h>
unsigned int do_IRQ(struct pt_regs *regs)
{
    struct pt_regs *old_regs = set_irq_regs(regs);

	unsigned vector = ~regs->orig_ax;
	unsigned irq;

	exit_idle();
	irq_enter();

	early_print_str(itoah(buf_in_doIRQ, counter_in_doIRQ));

	irq = __this_cpu_read(vector_irq[vector]);

	if (!handle_irq(irq, regs)) {
        ack_APIC_irq();
	}

	irq_exit();

	set_irq_regs(old_regs);
	return 1;
}
