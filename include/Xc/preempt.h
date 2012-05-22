#ifndef _XC_PREEMPT_H
#define _XC_PREEMPT_H

#include <Xc/thread_info.h>

void preempt_schedule(void);

#define preempt_count() (current_thread_info()->preempt_count)

#define add_preempt_count(val) do { preempt_count() += (val); } while (0)
#define sub_preempt_count(val) do { preempt_count() -= (val); } while (0)

#define add_preempt_count_notrace(val)    \
	do { preempt_count() += (val); } while (0)
#define sub_preempt_count_notrace(val)    \
	do { preempt_count() -= (val); } while (0)

#define inc_preempt_count() add_preempt_count(1)
#define dec_preempt_count() sub_preempt_count(1)

#define preempt_check_resched()    \
	do {                           \
		if (test_thread_flag(TIF_NEED_RESCHED))   \
		    preempt_schedule();    \
	} while (0)

#define preempt_disable()    \
	do {                     \
		inc_preempt_count(); \
		barrier();           \
	} while (0)


#define preempt_enable_no_resched()    \
	do {                               \
		barrier();                     \
		dec_preempt_count();           \
	} while (0)

#define preempt_enable()    \
	do {                    \
		preempt_enable_no_resched();   \
		barrier();          \
		preempt_check_resched();       \
	} while (0)


#endif
