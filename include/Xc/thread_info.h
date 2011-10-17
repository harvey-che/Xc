#ifndef _XC_THREAD_INFO_H
#define _XC_THREAD_INFO_H

#ifndef __ASSEMBLY__

#include <types.h>

#define PREEMPT_ACTIVE 0x10000000

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

#endif

#endif
