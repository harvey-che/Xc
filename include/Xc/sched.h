#ifndef _XC_SCHED_H
#define _XC_SCHED_H

#include <asm/pgtable_types.h>
#include <Xc/thread_info.h>

union thread_union {
    struct thread_info thread_info;
	unsigned long stack[THREAD_SIZE/sizeof(long)];
};

extern union thread_union init_thread_union;

struct task_struct {
    volatile long state;
    void *stack;

};

#define task_thread_info(task) ((struct thread_info*)(task)->stack)

#define task_stack_page(task) ((task)->stack)

#define INIT_PREEMPT_COUNT (1 + PREEMPT_ACTIVE)

#endif
