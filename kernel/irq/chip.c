#include <Xc/spinlock.h>
#include <Xc/wait.h>
#include <Xc/irq.h>
#include <Xc/irqdesc.h>
#include <Xc/errno.h>
#include "internals.h"


int irq_set_chip(unsigned int irq, struct irq_chip *chip)
{
    unsigned long flags;
	struct irq_desc *desc = irq_get_desc_lock(irq, &flags);

	if (!desc)
		return -EINVAL;

	if (!chip)
		chip = &no_irq_chip;

	desc->irq_data.chip = chip;
	irq_put_desc_unlock(desc, flags);

	/* By harvey, what does this mean? */
	/* irq_reserve_irq(irq); */
	return 0;
}

static void irq_state_clr_disabled(struct irq_desc *desc)
{
    irqd_clear(&desc->irq_data, IRQD_IRQ_DISABLED);
}

static void irq_state_set_disabled(struct irq_desc *desc)
{
    irqd_set(&desc->irq_data, IRQD_IRQ_DISABLED);
}

static void irq_state_clr_masked(struct irq_desc *desc)
{
    irqd_set(&desc->irq_data, IRQD_IRQ_MASKED);
}

static void irq_state_set_masked(struct irq_desc *desc)
{
    irqd_set(&desc->irq_data, IRQD_IRQ_MASKED);
}

static bool irq_check_poll(struct irq_desc *desc)
{
    if (!(desc->istate & IRQS_POLL_INPROGRESS))
		return false;
	return irq_wait_for_poll(desc);
}

static inline void mask_ack_irq(struct irq_desc *desc)
{
    if (desc->irq_data.chip->irq_mask_ack)
		desc->irq_data.chip->irq_mask_ack(&desc->irq_data);
	else {
        desc->irq_data.chip->irq_mask(&desc->irq_data);
		if (desc->irq_data.chip->irq_ack)
			desc->irq_data.chip->irq_ack(&desc->irq_data);
	}
	irq_state_set_masked(desc);
}

void mask_irq(struct irq_desc *desc)
{
    if (desc->irq_data.chip->irq_mask) {
        desc->irq_data.chip->irq_mask(&desc->irq_data);
		irq_state_set_masked(desc);
	}
}

void unmask_irq(struct irq_desc *desc)
{
    if (desc->irq_data.chip->irq_unmask) {
        desc->irq_data.chip->irq_unmask(&desc->irq_data);
		irq_state_clr_masked(desc);
	}
}

int irq_startup(struct irq_desc *desc)
{
    irq_state_clr_disabled(desc);
	desc->depth = 0;

	if (desc->irq_data.chip->irq_startup) {
        int ret = desc->irq_data.chip->irq_startup(&desc->irq_data);
		irq_state_clr_masked(desc);
		return ret;
	}

	irq_enable(desc);
	return 0;
}

void irq_enable(struct irq_desc *desc)
{
    irq_state_clr_disabled(desc);
	if (desc->irq_data.chip->irq_enable)
		desc->irq_data.chip->irq_enable(&desc->irq_data);
	else
		desc->irq_data.chip->irq_unmask(&desc->irq_data);
	irq_state_clr_masked(desc);
}

void irq_disable(struct irq_desc *desc)
{
    irq_state_set_disabled(desc);
	if (desc->irq_data.chip->irq_disable) {
        desc->irq_data.chip->irq_disable(&desc->irq_data);
		irq_state_set_masked(desc);
	}
}

void __irq_set_handler(unsigned int irq, irq_flow_handler_t handle, int is_chained, const char *name)
{
    unsigned long flags;

	struct irq_desc *desc = irq_get_desc_buslock(irq, &flags);

	if (!desc)
		return;

	if (!handle) {
        handle = handle_bad_irq;
	} else {
        if (desc->irq_data.chip == &no_irq_chip)
			goto out;
	}

	if (handle == handle_bad_irq) {
        if (desc->irq_data.chip != &no_irq_chip)
			mask_ack_irq(desc);
		irq_state_set_disabled(desc);
		desc->depth = 1;
	}
    desc->handle_irq = handle;
	desc->name = name;
 
	if (handle != handle_bad_irq && is_chained) {
        irq_settings_set_noprobe(desc);
		irq_settings_set_norequest(desc);
		irq_settings_set_nothread(desc);
		irq_startup(desc);
	}
out:
	irq_put_desc_busunlock(desc, flags);
}

void irq_set_chip_and_handler_name(unsigned int irq, struct irq_chip *chip, irq_flow_handler_t handle, const char *name)
{
    irq_set_chip(irq, chip);
	__irq_set_handler(irq, handle, 0, name);
}

void handle_level_irq(unsigned int irq, struct irq_desc *desc)
{
    raw_spin_lock(&desc->lock);
	mask_ack_irq(desc);

	if (irqd_irq_inprogress(&desc->irq_data))
		if (!irq_check_poll(desc))
			goto out_unlock;

	desc->istate &= ~(IRQS_REPLAY | IRQS_WAITING);
	/* kstat_incr_irqs_this_cpu(irq, desc); */

	if (!desc->action || irqd_irq_disabled(&desc->irq_data))
		goto out_unlock;

	handle_irq_event(desc);

	if (!irqd_irq_disabled(&desc->irq_data) && !(desc->istate & IRQS_ONESHOT))
		unmask_irq(desc);
out_unlock:
	raw_spin_unlock(&desc->lock);
}
