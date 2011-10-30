#include <Xc/sched.h>
#include <Xc/preempt.h>
#include <Xc/irqflags.h>



static void __schedule(void)
{

}

static void __cond_resched(void)
{
    add_preempt_count(PREEMPT_ACTIVE);
	__schedule();
	sub_preempt_count(PREEMPT_ACTIVE);
}

static inline int should_resched(void)
{
    return need_resched() && !(preempt_count() & PREEMPT_ACTIVE);
}

int _cond_resched(void)
{
    if (should_resched()) {
        __cond_resched();
		return 1;
	}
	return 0;
}

void preempt_schedule(void)
{
    struct thread_info *ti = current_thread_info();

	if (ti->preempt_count || irqs_disabled())
		return;

	do {
		add_preempt_count_notrace(PREEMPT_ACTIVE);
		__schedule();
		sub_preempt_count_notrace(PREEMPT_ACTIVE);
		
		barrier();
	} while (need_resched());
}
