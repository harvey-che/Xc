#include <Xc/init.h>
#include <Xc/compiler-gcc.h>
#include <Xc/hardirq.h>
#include <Xc/preempt.h>
#include <Xc/irqflags.h>
#include <Xc/tick.h>
#include <Xc/interrupt.h>
#include <Xc/smp.h>
#include <Xc/sched.h>

static inline void __local_bh_disable(unsigned long ip, unsigned int cnt)
{
    add_preempt_count(cnt);
	barrier();
}

void local_bh_disable(void)
{
    __local_bh_disable((unsigned long)__builtin_return_address(0), SOFTIRQ_DISABLE_OFFSET);
}

static void __local_bh_enable(unsigned int cnt)
{
    /*
	 *  WARN_ON_ONCE(in_irq());
	 *  WARN_ON_ONCE(!irqs_disabled());
	 */

	if (softirq_count() == cnt)
		trace_softirqs_on((unsigned long)__builtin_return_address(0));
	sub_preempt_count(cnt);
}

void _local_bh_enable(void)
{
    __local_bh_enable(SOFTIRQ_DISABLE_OFFSET);
}

void irq_enter(void)
{
    int cpu = smp_processor_id();

	rcu_irq_enter();
    if (idle_cpu(cpu) && !in_interrupt()) {
        local_bh_disable();
		tick_check_idle(cpu);
		_local_bh_enable();
	}

	__irq_enter();
}

void irq_exit(void)
{
    account_system_vtime(current);
	trace_hardirq_exit();
	sub_preempt_count(IRQ_EXIT_OFFSET);
	/*
	if (!in_interrupt() && local_softirq_pending())
		invoke_softirq();
	*/
	rcu_irq_exit();

	preempt_enable_no_resched();
}

int __init __weak arch_early_irq_init(void)
{
    return 0;
}

