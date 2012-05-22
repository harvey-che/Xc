#include <stddef.h>
#include <Xc/smp.h>
#include <Xc/atomic.h>
#include <Xc/wait.h>
#include <Xc/spinlock.h>
#include <Xc/cpumask.h>
#include <Xc/irq.h>
#include <Xc/irqdesc.h>

int noirqdebug;

static int irq_poll_cpu;

bool irq_wait_for_poll(struct irq_desc *desc)
{
    if (irq_poll_cpu == smp_processor_id())
		return false;
#ifdef CONFIG_SMP

#else
	return false;
#endif
}
