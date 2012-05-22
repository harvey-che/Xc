#include <Xc/irqdesc.h>

#include "settings.h"


#define IRQ_BITMAP_BITS NR_IRQS

#define istate core_internal_state_do_not_mess_with_it

enum {
    IRQTF_RUNTHREAD,
	IRQTF_DIED,
	IRQTF_WARNED,
	IRQTF_AFFINITY,
	IRQTF_FORCED_THREAD,
};

enum {
    IRQS_AUTODETECT    = 0x00000001,
	IRQS_SPURIOUS_DISABLED = 0x00000002,
	IRQS_POLL_INPROGRESS   = 0x00000008,
	IRQS_ONESHOT           = 0x00000020,
	IRQS_REPLAY            = 0x00000040,
	IRQS_WAITING           = 0x00000080,
	IRQS_PENDING           = 0x00000200,
	IRQS_SUSPENDED         = 0x00000800,
};

extern int noirqdebug;

static inline void irqd_clear(struct irq_data *d, unsigned int mask)
{
    d->state_use_accessors &= ~mask;
}

static inline void irqd_set(struct irq_data *d, unsigned int mask)
{
    d->state_use_accessors |= mask;
}

extern int irq_startup(struct irq_desc *desc);
extern void irq_shutdown(struct irq_desc *desc);
extern void irq_enable(struct irq_desc *desc);
extern void irq_disable(struct irq_desc *desc);
extern void mask_irq(struct irq_desc *desc);
extern void unmask_irq(struct irq_desc *desc);

#define irq_data_to_desc(data) container_of(data, struct irq_desc, irq_data)

irqreturn_t handle_irq_event_percpu(struct irq_desc *desc, struct irqaction *action);
irqreturn_t handle_irq_event(struct irq_desc *desc);

bool irq_wait_for_poll(struct irq_desc *desc);

static inline void chip_bus_lock(struct irq_desc *desc)
{
    if (desc->irq_data.chip->irq_bus_lock)
		desc->irq_data.chip->irq_bus_lock(&desc->irq_data);
}

static inline void chip_bus_sync_unlock(struct irq_desc *desc)
{
    if (desc->irq_data.chip->irq_bus_sync_unlock)
		desc->irq_data.chip->irq_bus_sync_unlock(&desc->irq_data);
}

struct irq_desc *__irq_get_desc_lock(unsigned int irq, unsigned long *flags, bool bus);

void __irq_put_desc_unlock(struct irq_desc *desc, unsigned long flags, bool bus);

static inline struct irq_desc *irq_get_desc_buslock(unsigned int irq, unsigned long *flags)
{
    return __irq_get_desc_lock(irq, flags, true);
}

static inline void irq_put_desc_busunlock(struct irq_desc *desc, unsigned long flags)
{
    __irq_put_desc_unlock(desc, flags, true);
}

static inline struct irq_desc *irq_get_desc_lock(unsigned int irq, unsigned long *flags)
{
    return __irq_get_desc_lock(irq, flags, false);
}

static inline void irq_put_desc_unlock(struct irq_desc *desc, unsigned long flags)
{
    __irq_put_desc_unlock(desc, flags, false);
}
