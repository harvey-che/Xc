#include <Xc/wait.h>
#include <Xc/atomic.h>
#include <Xc/bitops.h>
#include <Xc/cpumask.h>
#include <Xc/irq.h>
#include <Xc/irqdesc.h>
#include <Xc/interrupt.h>
#include "internals.h"


irqreturn_t no_action(int cpl, void *dev_id)
{
	return IRQ_NONE;
}

void handle_bad_irq(unsigned int irq, struct irq_desc *desc)
{
    /* By harvey, Not Implemented right now */
}

static void warn_no_thread(unsigned int irq, struct irqaction *action)
{
    if(test_and_set_bit(IRQTF_WARNED, &action->thread_flags))
		return;
}

static void irq_wake_thread(struct irq_desc *desc, struct irqaction *action)
{
    if (test_bit(IRQTF_DIED, &action->thread_flags) || test_and_set_bit(IRQTF_RUNTHREAD, &action->thread_flags))
		return;

	desc->threads_oneshot |= action->thread_mask;
	/* wake_up_process(action->thread); */
}

irqreturn_t handle_irq_event_percpu(struct irq_desc *desc, struct irqaction *action)
{
    irqreturn_t retval = IRQ_NONE;
	unsigned int random = 0, irq = desc->irq_data.irq;

	do {
		irqreturn_t res;

		/* trace_irq_handler_entry(irq, action); */
		res = action->handler(irq, action->dev_id);
		/* trace_irq_handler_exit(irq, action, res); */

		if (!irqs_disabled())
			local_irq_disable();

		switch(res) {
        case IRQ_WAKE_THREAD:
			if (!action->thread_fn) {
				warn_no_thread(irq, action);
				break;
			}

			irq_wake_thread(desc, action);
		case IRQ_HANDLED:
			random |= action->flags;
			break;
		default:
			break;
		}

		retval |= res;
		action = action->next;
	} while (action);
    /*
	if (random & IRQF_SAMPLE_RANDOM)
		add_interrupt_randomness(irq);
    */
	/*
	if (!noirqdebug)
		note_iterrupt(irq, desc, retval);
	*/
	return retval;

}

irqreturn_t handle_irq_event(struct irq_desc *desc)
{
    struct irqaction *action = desc->action;
	irqreturn_t ret;

	desc->istate &= ~IRQS_PENDING;
	irqd_set(&desc->irq_data, IRQD_IRQ_INPROGRESS);
	raw_spin_unlock(&desc->lock);

	ret = handle_irq_event_percpu(desc, action);
	
	raw_spin_lock(&desc->lock);
	irqd_clear(&desc->irq_data, IRQD_IRQ_INPROGRESS);
	return ret;
}
