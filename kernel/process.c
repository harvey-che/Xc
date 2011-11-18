#include <Xc/gfp.h>

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
