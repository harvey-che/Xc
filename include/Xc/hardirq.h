#ifndef _XC_HARDIRQ_H
#define _XC_HARDIRQ_H

/* asm/hardirq.h */
extern void ack_bad_irq(unsigned int irq);
/* end -- asm/hardirq.h */

#define PREEMPT_BITS 8
#define SOFTIRQ_BITS 8
#define NMI_BITS     1

#define MAX_HARDIRQ_BITS 10

#define HARDIRQ_BITS MAX_HARDIRQ_BITS

#define PREEMPT_SHIFT 0
#define SOFTIRQ_SHIFT (PREEMPT_SHIFT + PREEMPT_BITS)
#define HARDIRQ_SHIFT (SOFTIRQ_SHIFT + SOFTIRQ_BITS)
#define NMI_SHIFT     (HARDIRQ_SHIFT + HARDIRQ_BITS)

#define __IRQ_MASK(x)  ((1UL << (x)) - 1)

#define PREEMPT_MASK (__IRQ_MASK(PREEMPT_BITS) << PREEMPT_SHIFT)
#define SOFTIRQ_MASK (__IRQ_MASK(SOFTIRQ_BITS) << SOFTIRQ_SHIFT)
#define HARDIRQ_MASK (__IRQ_MASK(HARDIRQ_BITS) << HARDIRQ_SHIFT)
#define NMI_MASK     (__IRQ_MASK(NMI_BITS)     << NMI_SHIFT)

#define PREEMPT_OFFSET (1UL << PREEMPT_SHIFT)
#define SOFTIRQ_OFFSET (1UL << SOFTIRQ_SHIFT)
#define HARDIRQ_OFFSET (1UL << HARDIRQ_SHIFT)
#define NMI_OFFSET     (1UL << NMI_SHIFT)

#define SOFTIRQ_DISABLE_OFFSET (2 * SOFTIRQ_OFFSET)

#define hardirq_count() (preempt_count() & HARDIRQ_MASK)
#define softirq_count() (preempt_count() & SOFTIRQ_MASK)
#define irq_count()   (preempt_count() & (HARDIRQ_MASK | SOFTIRQ_MASK | NMI_MASK))

#define in_irq()      (hardirq_count())
#define in_softirq()  (softirq_count())
#define in_interrupt() (irq_count())

#define IRQ_EXIT_OFFSET (HARDIRQ_OFFSET - 1)

#define rcu_irq_enter() do {} while (0)
#define rcu_irq_exit() do {} while (0)

#include <Xc/sched.h>

static inline void account_system_vtime(struct task_struct *tsk)
{
}

extern void irq_enter(void);
extern void irq_exit(void);

#define __irq_enter()    \
	do {                 \
		account_system_vtime(current);        \
		add_preempt_count(HARDIRQ_OFFSET);    \
		trace_hardirq_enter();                \
	} while (0)

#endif
