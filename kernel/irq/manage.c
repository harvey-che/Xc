#include <Xc/cpumask.h>
#include <Xc/irq.h>
#include <Xc/wait.h>
#include <Xc/irqdesc.h>
#include <Xc/interrupt.h>
#include <Xc/module.h>
#include <Xc/errno.h>
#include <Xc/gfp.h>
#include "internals.h"

/* irq/resend.c */
void check_irq_resend(struct irq_desc *desc, unsigned irq)
{
    if (irq_settings_is_level(desc))
		return;

	if (desc->istate & IRQS_REPLAY)
		return;

	if (desc->istate & IRQS_PENDING) {
        desc->istate &= ~IRQS_PENDING;
		desc->istate |= IRQS_REPLAY;

		if (!desc->irq_data.chip->irq_retrigger ||
			!desc->irq_data.chip->irq_retrigger(&desc->irq_data)) {
            /* set_bit(irq, irqs_resend);
			tasklet_schedule(&resend_tasklet); */
		}
	}
}
/* */

static inline int setup_affinity(unsigned int irq, struct irq_desc *desc, struct cpumask *mask)
{
    return 0;
}

static irqreturn_t irq_default_primary_handler(int irq, void *dev_id)
{
    return IRQ_WAKE_THREAD;
}

static void irq_setup_forced_threading(struct irqaction *new)
{
    if (!force_irqthreads)
		return;

	if (new->flags & (IRQF_NO_THREAD | IRQF_PERCPU | IRQF_ONESHOT))
		return;

	new->flags |= IRQF_ONESHOT;

	if (!new->thread_fn) {
        set_bit(IRQTF_FORCED_THREAD, &new->thread_flags);
		new->thread_fn = new->handler;
		new->handler = irq_default_primary_handler;
	}
}

int __irq_set_trigger(struct irq_desc *desc, unsigned int irq, unsigned long flags)
{
    struct irq_chip *chip = desc->irq_data.chip;
	int ret, unmask = 0;

	if (!chip || !chip->irq_set_type) {
        return 0;
	}

	flags &= IRQ_TYPE_SENSE_MASK;

	if (chip->flags & IRQCHIP_SET_TYPE_MASKED) {
        if (!irqd_irq_masked(&desc->irq_data))
			mask_irq(desc);
		if (!irqd_irq_disabled(&desc->irq_data))
			unmask = 1;
	}

	ret = chip->irq_set_type(&desc->irq_data, flags);

	switch(ret) {
		case IRQ_SET_MASK_OK:
	    irqd_clear(&desc->irq_data, IRQD_TRIGGER_MASK);
		irqd_set(&desc->irq_data, flags);

	case IRQ_SET_MASK_OK_NOCOPY:
		flags = irqd_get_trigger_type(&desc->irq_data);
		irq_settings_set_trigger_mask(desc, flags);
		irqd_clear(&desc->irq_data, IRQD_LEVEL);
		irq_settings_clr_level(desc);
		if (flags & IRQ_TYPE_LEVEL_MASK) {
            irq_settings_set_level(desc);
			irqd_set(&desc->irq_data, IRQD_LEVEL);
		}
		ret = 0;
		break;
	default:
		;
	}
	if (unmask)
		unmask_irq(desc);
	return ret;
}

static irqreturn_t irq_nested_primary_handler(int irq, void *dev_id)
{
    /* WARN(1, "Primary handler called for nested irq %d\n", irq); */
	return IRQ_NONE;
}

void __disable_irq(struct irq_desc *desc, unsigned int irq, bool suspend)
{
    if (suspend) {
        if (!desc->action || (desc->action->flags & IRQF_NO_SUSPEND))
			return;
		desc->istate |= IRQS_SUSPENDED;
	}

	if (!desc->depth++)
		irq_disable(desc);
}

static int __disable_irq_nosync(unsigned int irq)
{
    unsigned long flags;
	struct irq_desc *desc = irq_get_desc_buslock(irq, &flags);

	if (!desc)
		return -EINVAL;
	__disable_irq(desc, irq, false);
	irq_put_desc_busunlock(desc, flags);
	return 0;
}

void disable_irq_nosync(unsigned int irq)
{
    __disable_irq_nosync(irq);
}

void __enable_irq(struct irq_desc *desc, unsigned int irq, bool resume)
{
    if (resume) {
        if (!(desc->istate & IRQS_SUSPENDED)) {
            if (!desc->action)
				return;

			if (!(desc->action->flags & IRQF_FORCE_RESUME))
				return;

			desc->depth++;
		}
		desc->istate &= ~IRQS_SUSPENDED;
	}

	switch(desc->depth) {
    case 0:
err_out:
		break;
	case 1: {
		if (desc->istate & IRQS_SUSPENDED)
			goto err_out;
		irq_settings_set_noprobe(desc);
		irq_enable(desc);
		check_irq_resend(desc, irq);
	}
	default:
		desc->depth--;
	}
}

void enable_irq(unsigned int irq)
{
    unsigned long flags;
	struct irq_desc *desc = irq_get_desc_buslock(irq, &flags);

	if (!desc)
		return;

	if (!desc->irq_data.chip)
		goto out;
    
	__enable_irq(desc, irq, false);
out:
	irq_put_desc_busunlock(desc, flags);
}

static int __setup_irq(unsigned int irq, struct irq_desc *desc, struct irqaction *new)
{
    struct irqaction *old, **old_ptr;
	const char *old_name = NULL;
	unsigned long flags, thread_mask = 0;
	int ret, nested, shared = 0;
	cpumask_var_t mask;

	if (!desc)
		return -EINVAL;

	if (desc->irq_data.chip == &no_irq_chip)
		return -ENOSYS;

	if (!try_module_get(desc->owner))
		return -ENODEV;

	/* By harvey, comment this temporarily */
	/*
	if (new->flags & IRQF_SAMPLE_RANDOM) {
        rand_initialize_irq(irq);
	}
    */

	nested = irq_settings_is_nested_thread(desc);
	if (nested) {
        if (!new->thread_fn) {
            ret = -EINVAL;
			goto out_mput;
		}

		new->handler = irq_nested_primary_handler;
	} else {
        if (irq_settings_can_thread(desc))
			irq_setup_forced_threading(new);
	}

	if (new->thread_fn && !nested) {
		/*
        struct task_struct *t;

		t = kthread_create(irq_thread, new, "irq/%d-%s", irq, new->name);
		if (IS_ERR(t)) {
            ret = PTR_ERR(t);
			goto out_mput;
		}

		get_task_struct(t);
		new->thread = t;
		*/
	}

	if (!alloc_cpumask_var(&mask, GFP_KERNEL)) {
        ret = -ENOMEM;
		goto out_thread;
	}

	raw_spin_lock_irqsave(&desc->lock, flags);
	old_ptr = &desc->action;
	old = *old_ptr;

	if (old) {
        if (!((old->flags & new->flags) & IRQF_SHARED) || 
			((old->flags ^ new->flags) & IRQF_TRIGGER_MASK) ||
			((old->flags ^ new->flags) & IRQF_ONESHOT)) {
            old_name = old->name;
			goto mismatch;
		}

		if ((old->flags & IRQF_PERCPU) != (new->flags & IRQF_PERCPU))
			goto mismatch;

		do {
            thread_mask |= old->thread_mask;
			old_ptr = &old->next;
			old = *old_ptr;
		} while (old);

		shared = 1;
	}

	if (new->flags & IRQF_ONESHOT && thread_mask == ~0UL) {
        ret = -EBUSY;
		goto out_mask;
	}
	new->thread_mask = 1 << ffz(thread_mask);

	if (!shared) {
        init_waitqueue_head(&desc->wait_for_threads);

		if (new->flags & IRQF_TRIGGER_MASK) {
            ret = __irq_set_trigger(desc, irq, new->flags &IRQF_TRIGGER_MASK);

			if (ret)
				goto out_mask;
		}

		desc->istate &= ~(IRQS_AUTODETECT | IRQS_SPURIOUS_DISABLED | 
				          IRQS_ONESHOT | IRQS_WAITING);

		irqd_clear(&desc->irq_data, IRQD_IRQ_INPROGRESS);

		if (new->flags & IRQF_PERCPU) {
            irqd_set(&desc->irq_data, IRQD_PER_CPU);
			irq_settings_set_per_cpu(desc);
		}

		if (new->flags & IRQF_ONESHOT)
			desc->istate |= IRQS_ONESHOT;

		if (irq_settings_can_autoenable(desc))
			irq_startup(desc);
		else
			desc->depth = 1;

		if (new->flags & IRQF_NOBALANCING) {
            irq_settings_set_no_balancing(desc);
			irqd_set(&desc->irq_data, IRQD_NO_BALANCING);
		}

		setup_affinity(irq, desc, mask);
	} else if (new->flags & IRQF_TRIGGER_MASK) {
        unsigned int nmsk = new->flags & IRQF_TRIGGER_MASK;
		unsigned int omsk = irq_settings_get_trigger_mask(desc);

		if (nmsk != omsk) {

		}
	}

	new->irq = irq;
	*old_ptr = new;

	desc->irq_count = 0;
	desc->irqs_unhandled = 0;

	if (shared && (desc->istate & IRQS_SPURIOUS_DISABLED)) {
        desc->istate &= ~IRQS_SPURIOUS_DISABLED;
		__enable_irq(desc, irq, false);
	}


	raw_spin_unlock_irqrestore(&desc->lock, flags);

	/*
	if (new->thread)
		wake_up_process(new->thread);
	*/

	/* register_irq_proc(irq, desc); */
	new->dir = NULL;
	/* register_handler_proc(irq, new); */
	free_cpumask_var(mask);

	return 0;

mismatch:
	ret = -EBUSY;
out_mask:
	raw_spin_unlock_irqrestore(&desc->lock, flags);
	free_cpumask_var(mask);
out_thread:
	if (new->thread) {
		/*
        struct task_sturct *t = new->thread;

		new->thread = NULL;
        
		if (!test_bit(IRQTF_DIED, &new->thread_flags))
		    kthread_stop(t);
		
		put_task_struct(t);
		*/
	}
out_mput:
	module_put(desc->owner);
	return ret;
}

int setup_irq(unsigned int irq, struct irqaction *act)
{
    int retval;
	struct irq_desc *desc = irq_to_desc(irq);

	chip_bus_lock(desc);
	retval = __setup_irq(irq, desc, act);
	chip_bus_sync_unlock(desc);

	return retval;
}
