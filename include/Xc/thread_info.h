#ifndef _XC_THREAD_INFO_H
#define _XC_THREAD_INFO_H

/*
 *
 */

#ifndef __ASSEMBLY__

#include <Xc/types.h>
#include <asm/page_types.h>
#include <Xc/bitops.h>

#define PREEMPT_ACTIVE 0x10000000

#define TIF_NEED_RESCHED    3

struct task_struct;

struct thread_info {
    struct task_struct *task;
	__u32 flags;
	__u32 status;
	int preempt_count;
	unsigned long previous_esp;
	__u8 supervisor_stack[0];
};

#define INIT_THREAD_INFO(tsk)    \
{                    \
	.task = &tsk,    \
    .flags = 0,    \
    .preempt_count = INIT_PREEMPT_COUNT,    \
}

#define init_thread_info (init_thread_union.thread_info)
#define init_stack (init_thread_union.stack)

register unsigned long current_stack_pointer asm("esp");

static inline struct thread_info *current_thread_info(void)
{
    return (struct thread_info*)(current_stack_pointer & ~(THREAD_SIZE - 1));
}

static inline int test_ti_thread_flag(struct thread_info *ti, int flag)
{
    return test_bit(flag, (unsigned long *)&ti->flags);
}

#define test_thread_flag(flag)    \
	test_ti_thread_flag(current_thread_info(), flag)



#endif

#endif
