#ifndef _XC_IRQ_H
#define _XC_IRQ_H

#include <Xc/spinlock.h>
#include <Xc/types.h>
#include <Xc/cpumask.h>
#include <asm/ptrace.h>
#include <Xc/interrupt.h>
#include <Xc/percpu.h>

/* asm/irq.h */
extern void native_init_IRQ(void);

extern void irq_ctx_init(int cpu);
extern void native_init_IRQ(void);
extern bool handle_irq(unsigned irq, struct pt_regs *regs);

extern unsigned int do_IRQ(struct pt_regs *regs);
extern void init_ISA_irqs(void);
/* end -- asm/irq.h */

/* asm/irq_regs.h */
DECLARE_PER_CPU(struct pt_regs *, irq_regs);

static inline struct pt_regs *get_irq_regs(void)
{
    return percpu_read(irq_regs);
}

static inline struct pt_regs *set_irq_regs(struct pt_regs *new_regs)
{
    struct pt_regs *old_regs;

	old_regs = get_irq_regs();
	percpu_write(irq_regs, new_regs);

	return old_regs;
}
/* end -- asm/irq_regs.h */

/* linux/irqnr.h */

extern struct irq_desc *irq_to_desc(unsigned int irq);

/*
#define nr_irqs             NR_IRQS
#define irq_to_desc(irq)    (&irq_desc[irq])
*/

/* end -- linux/irqnr.h */

struct irq_chip;
struct irq_domain;
struct irq_msi_desc;
struct seg_file;
struct irq_desc;

typedef void (*irq_flow_handler_t)(unsigned int irq, struct irq_desc *desc);

enum {
    IRQ_TYPE_NONE         = 0x00000000,
	IRQ_TYPE_EDGE_RISING  = 0x00000001,
	IRQ_TYPE_EDGE_FALLING = 0x00000002,
	IRQ_TYPE_EDGE_BOTH    = (IRQ_TYPE_EDGE_FALLING | IRQ_TYPE_EDGE_RISING),
	IRQ_TYPE_LEVEL_HIGH   = 0x00000004,
	IRQ_TYPE_LEVEL_LOW    = 0x00000008,
	IRQ_TYPE_LEVEL_MASK   = (IRQ_TYPE_LEVEL_LOW | IRQ_TYPE_LEVEL_HIGH),
	IRQ_TYPE_SENSE_MASK   = 0x0000000f,

	IRQ_TYPE_PROBE        = 0x00000010,

	IRQ_LEVEL             = (1 << 8),
	IRQ_PER_CPU           = (1 << 9),
	IRQ_NOPROBE           = (1 << 10),
	IRQ_NOREQUEST         = (1 << 11),
	IRQ_NOAUTOEN          = (1 << 12),
	IRQ_NO_BALANCING      = (1 << 13),
	IRQ_MOVE_PCNTXT       = (1 << 14),
	IRQ_NESTED_THREAD     = (1 << 15),
	IRQ_NOTHREAD          = (1 << 16),
};

#define IRQF_MODIFY_MASK    \
	(IRQ_TYPE_SENSE_MASK | IRQ_NOPROBE | IRQ_NOREQUEST |    \
	 IRQ_NOAUTOEN | IRQ_MOVE_PCNTXT | IRQ_LEVEL | IRQ_NO_BALANCING |    \
	 IRQ_PER_CPU | IRQ_NESTED_THREAD)

#define IRQ_NO_BALANCING_MASK (IRQ_PER_CPU | IRQ_NO_BALANCING)

enum {
    IRQ_SET_MASK_OK = 0,
	IRQ_SET_MASK_OK_NOCOPY,
};


struct irq_data {
    unsigned int     irq;
	unsigned long    hwirq;
	unsigned int     node;
	unsigned int     state_use_accessors;
	struct irq_chip  *chip;
	struct irq_domain *domain;
	void              *handler_data;
	void              *chip_data;
	struct msi_desc   *msi_desc;
#ifdef CONFIG_SMP
	cpumask_var_t     affinity;
#endif
};

enum {
    IRQD_TRIGGER_MASK        = 0xf,
	IRQD_SETAFFINITY_PENDING = (1 << 8),
	IRQD_NO_BALANCING        = (1 << 10),
	IRQD_PER_CPU             = (1 << 11),
	IRQD_AFFINITY_SET        = (1 << 12),
	IRQD_LEVEL               = (1 << 13),
	IRQD_WAKEUP_STATE        = (1 << 14),
	IRQD_MOVE_PCNTXT         = (1 << 15),
	IRQD_IRQ_DISABLED        = (1 << 16),
	IRQD_IRQ_MASKED          = (1 << 17),
	IRQD_IRQ_INPROGRESS      = (1 << 18),
};

static inline u32 irqd_get_trigger_type(struct irq_data *d)
{
    return d->state_use_accessors & IRQD_TRIGGER_MASK;
}

static inline bool irqd_irq_disabled(struct irq_data *d)
{
    return d->state_use_accessors & IRQD_IRQ_DISABLED;
}

static inline bool irqd_irq_masked(struct irq_data *d)
{
    return d->state_use_accessors & IRQD_IRQ_MASKED;
}

static inline bool irqd_irq_inprogress(struct irq_data *d)
{
    return d->state_use_accessors & IRQD_IRQ_INPROGRESS;
}

struct irq_chip {
	const char	*name;
	unsigned int	(*irq_startup)(struct irq_data *data);
	void		(*irq_shutdown)(struct irq_data *data);
	void		(*irq_enable)(struct irq_data *data);
	void		(*irq_disable)(struct irq_data *data);

	void		(*irq_ack)(struct irq_data *data);
	void		(*irq_mask)(struct irq_data *data);
	void		(*irq_mask_ack)(struct irq_data *data);
	void		(*irq_unmask)(struct irq_data *data);
	void		(*irq_eoi)(struct irq_data *data);

	int		(*irq_set_affinity)(struct irq_data *data, const struct cpumask *dest, bool force);
	int		(*irq_retrigger)(struct irq_data *data);
	int		(*irq_set_type)(struct irq_data *data, unsigned int flow_type);
	int		(*irq_set_wake)(struct irq_data *data, unsigned int on);

	void		(*irq_bus_lock)(struct irq_data *data);
	void		(*irq_bus_sync_unlock)(struct irq_data *data);

	void        (*irq_cpu_online)(struct irq_data *data);
	void        (*irq_cpu_offline)(struct irq_data *data);

	void        (*irq_suspend)(struct irq_data *data);
	void        (*irq_resume)(struct irq_data *data);
	void        (*irq_pm_shutdown)(struct irq_data *data);

	void        (*irq_print_chip)(struct irq_data *data, struct seg_file *p);

	unsigned long flags;

	/* Currently used only by UML, might disappear one day.*/
#ifdef CONFIG_IRQ_RELEASE_METHOD
	void		(*release)(unsigned int irq, void *dev_id);
#endif
};

enum {
    IRQCHIP_SET_TYPE_MASKED    = (1 << 0),
	IRQCHIP_EOI_IF_HANDLED     = (1 << 1),
	IRQCHIP_MASK_ON_SUSPEND    = (1 << 2),
	IRQCHIP_ONOFFLINE_ENABLED  = (1 << 3),
};

#define ARCH_IRQ_INIT_FLAGS 0

#define IRQ_DEFAULT_INIT_FLAGS ARCH_IRQ_INIT_FLAGS

extern int setup_irq(unsigned int irq, struct irqaction *new);

extern void handle_level_irq(unsigned int irq, struct irq_desc *desc);
extern void handle_bad_irq(unsigned int irq, struct irq_desc *desc);

extern struct irq_chip no_irq_chip;
extern struct irq_chip dummy_irq_chip;

extern void irq_set_chip_and_handler_name(unsigned int irq, struct irq_chip *chip, 
		                                  irq_flow_handler_t handle, const char *name);

extern int irq_set_chip(unsigned int irq, struct irq_chip *chip);

/*
static inline int irq_reserve_irq(unsigned int irq)
{
    return irq_reserve_irqs(irq, 1);
}
*/

#endif
