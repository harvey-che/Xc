#ifndef _XC_THREAD_INFO_H
#define _XC_THREAD_INFO_H

#include <Xc/types.h>
#include <asm/page_types.h>
#include <asm/processor.h>

#define PREEMPT_ACTIVE 0x10000000

#define THREAD_FLAGS (GFP_KERNEL | __GFP_NOTRACK)

#define TIF_SYSCALL_TRACE   0
#define TIF_SIGPENDING      2
#define TIF_NEED_RESCHED    3
#define TIF_SINGLESTEP      4
#define TIF_SYSCALL_EMU     6
#define TIF_SYSCALL_AUDIT   7
#define TIF_SECCOMP         8


#define _TIF_SYSCALL_TRACE    (1 << TIF_SYSCALL_TRACE)
#define _TIF_SINGLESTEP       (1 << TIF_SINGLESTEP)
#define _TIF_NEED_RESCHED     (1 << TIF_NEED_RESCHED)
#define _TIF_SYSCALL_EMU      (1 << TIF_SYSCALL_EMU)
#define _TIF_SYSCALL_AUDIT    (1 << TIF_SYSCALL_AUDIT)
#define _TIF_SECCOMP          (1 << TIF_SECCOMP)

#define _TIF_WORK_MASK    \
	(0x0000FFFF & ~(_TIF_SYSCALL_TRACE | _TIF_SYSCALL_AUDIT | _TIF_SINGLESTEP | \
	 _TIF_SECCOMP | _TIF_SYSCALL_EMU))

#define GET_THREAD_INFO(reg)    \
	movl $-THREAD_SIZE, reg;     \
    andl %esp, reg

#ifndef __ASSEMBLY__

#include <Xc/bitops.h>



struct task_struct;

struct thread_info {
    struct task_struct *task;
	__u32 flags;
	__u32 status;
	__u32 cpu;
	int preempt_count;

	mm_segment_t addr_limit;
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

#define alloc_thread_info_node(tsk, node)    \
	({                                       \
	     struct page *page = alloc_pages_node(node, THREAD_FLAGS,    \
			                                  THREAD_ORDER);         \
	     struct thread_info *ret = page ? page_address(page) : NULL; \
	     ret; })

register unsigned long current_stack_pointer asm("esp");

static inline struct thread_info *current_thread_info(void)
{
    return (struct thread_info*)(current_stack_pointer & ~(THREAD_SIZE - 1));
}

static inline int test_ti_thread_flag(struct thread_info *ti, int flag)
{
    return test_bit(flag, (unsigned long *)&ti->flags);
}

static inline void set_ti_thread_flag(struct thread_info *ti, int flag)
{
    set_bit(flag, (unsigned long *)&ti->flags);
}

static inline void clear_ti_thread_flag(struct thread_info *ti, int flag)
{
    clear_bit(flag, (unsigned long *)&ti->flags);
}


#define test_thread_flag(flag)    \
	test_ti_thread_flag(current_thread_info(), flag)

extern int arch_dup_task_struct(struct task_struct *dst, struct task_struct *src);
extern void free_thread_info(struct thread_info *ti);

#endif /* __ASSEMBLY__ */

#endif
