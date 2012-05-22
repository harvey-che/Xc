#include <Xc/gfp.h>
#include <Xc/sched.h>
#include <asm/ptrace.h>
#include <asm/processor.h>

int arch_dup_task_struct(struct task_struct *dst, struct task_struct *src)
{
    int ret;

	*dst = *src;

	/* Following is involving with FPU and commmented */
	return 0;
}

void free_thread_info(struct thread_info *ti)
{
    /* free_thread_xstate(ti->task); */
	free_pages((unsigned long)ti, get_order(THREAD_SIZE));
}

void arch_task_cache_init(void)
{
	/*
    task_xstate_cachep = kmem_cache_create("task_xstate", xstate_size, __alignof__(union thread_xstate), SLAB_PANIC | SLAB_NOTRACK, NULL);
	*/
}

int kernel_thread(int (*fn)(void *), void *arg, unsigned long flags)
{
    struct pt_regs regs;

	memset(&regs, 0, sizeof(regs));

	regs.si = (unsigned long)fn;
	regs.di = (unsigned long)arg;

	regs.ds = __USER_DS;
	regs.es = __USER_DS;
	regs.fs = __KERNEL_PERCPU;
	regs.gs = __KERNEL_STACK_CANARY;

	regs.orig_ax = -1;
	regs.ip = (unsigned long)kernel_thread_helper;
	regs.cs = __KERNEL_CS | get_kernel_rpl();
	regs.flags = X86_EFLAGS_IF | 0x2;

	return do_fork(flags | CLONE_VM | CLONE_UNTRACED, 0, &regs, 0, NULL, NULL);
}
