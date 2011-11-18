#include <Xc/init.h>
#include <Xc/errno.h>
#include <Xc/bitops.h>
#include <Xc/kernel.h>
#include <Xc/nodemask.h>
#include <Xc/wait.h>
#include <Xc/atomic.h>
#include <Xc/percpu.h>
#include <Xc/spinlock.h>
#include <asm/irq_vectors.h>
#include <Xc/irq.h>
#include <Xc/irqdesc.h>
#include <Xc/slab.h>
#include "internals.h"

struct irq_desc irq_desc[NR_IRQS] =  {
    [0 ... NR_IRQS - 1] = {
        .handle_irq = handle_bad_irq,
		.depth = 1,
		.lock = __RAW_SPIN_LOCK_UNLOCKED(irq_desc->lock),
	}
};

static void desc_set_defaults(unsigned int irq, struct irq_desc *desc, int node, struct module *owner)
{
    int cpu;

	desc->irq_data.irq = irq;
	desc->irq_data.chip = &no_irq_chip;
	desc->irq_data.chip_data = NULL;
	desc->irq_data.handler_data = NULL;
	desc->irq_data.msi_desc = NULL;
	irq_settings_clr_and_set(desc, ~0, _IRQ_DEFAULT_INIT_FLAGS);
	irqd_set(&desc->irq_data, IRQD_IRQ_DISABLED);
	desc->handle_irq = handle_bad_irq;
	desc->depth = 1;
	desc->irq_count = 0;
	desc->irqs_unhandled = 0;
	desc->name = NULL;
	desc->owner = owner;
	for_each_possible_cpu(cpu)
		*per_cpu_ptr(desc->kstat_irqs, cpu) = 0;
	/* desc_smp_init(desc, node); */
}

int __init early_irq_init(void)
{
    int count, i, node = first_online_node;
	struct irq_desc *desc;

	/* init_irq_default_affinity(); */

	desc = irq_desc;
	count = ARRAY_SIZE(irq_desc);

	for (i = 0; i < count; i++) {
        /* desc[i].kstat_irqs = alloc_percpu(unsigned int); */
        desc[i].kstat_irqs = kmalloc(sizeof(unsigned int), GFP_KERNEL);
		/* alloc_masks(&desc[i], GFP_KERNEL, node); */
		raw_spin_lock_init(&desc[i].lock);
		/* lockdep_set_class(&desc[i].lock, &irq_desc_lock_class); */
		desc_set_defaults(i, &desc[i], node, NULL);
	}

	return arch_early_irq_init();
}

struct irq_desc *irq_to_desc(unsigned int irq)
{
    return (irq < NR_IRQS) ? irq_desc + irq : NULL;
}

struct irq_desc *__irq_get_desc_lock(unsigned int irq, unsigned long *flags, bool bus)
{
    struct irq_desc *desc = irq_to_desc(irq);

	if (desc) {
        if (bus)
			chip_bus_lock(desc);
		raw_spin_lock_irqsave(&desc->lock, *flags);
	}
	return desc;
}

void __irq_put_desc_unlock(struct irq_desc *desc, unsigned long flags, bool bus)
{
    raw_spin_unlock_irqrestore(&desc->lock, flags);
	if (bus)
		chip_bus_sync_unlock(desc);
}
