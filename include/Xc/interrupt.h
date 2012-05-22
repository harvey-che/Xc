#ifndef _XC_INTERRUPT_H
#define _XC_INTERRUPT_H

#define IRQF_TRIGGER_NONOE    0x00000000
#define IRQF_TRIGGER_RISING   0x00000001
#define IRQF_TRIGGER_FALLING  0x00000002
#define IRQF_TRIGGER_HIGH     0x00000004
#define IRQF_TRIGGER_LOW      0x00000008
#define IRQF_TRIGGER_MASK     (IRQF_TRIGGER_HIGH | IRQF_TRIGGER_LOW | IRQF_TRIGGER_RISING | \
		                       IRQF_TRIGGER_FALLING)
#define IRQF_TRIGGER_PROBE    0x00000010

#define IRQF_DISABLED        0x00000020
#define IRQF_SAMPLE_RANDOM   0x00000040
#define IRQF_SHARED          0x00000080
#define IRQF_PROBE_SHARED    0x00000100
#define __IRQF_TIMER         0x00000200
#define IRQF_PERCPU          0x00000400
#define IRQF_NOBALANCING     0x00000800
#define IRQF_IRQPOLL         0x00001000
#define IRQF_ONESHOT         0x00002000
#define IRQF_NO_SUSPEND      0x00004000
#define IRQF_FORCE_RESUME    0x00008000
#define IRQF_NO_THREAD       0x00010000
#define IRQF_EARLY_RESUME    0x00020000

#define IRQF_TIMER    (__IRQF_TIMER | IRQF_NO_SUSPEND | IRQF_NO_THREAD)


struct proc_dir_entry;

enum irqreturn {
    IRQ_NONE    = (0 << 0),
	IRQ_HANDLED = (1 << 0),
	IRQ_WAKE_THREAD = (1 << 1),
};

typedef enum irqreturn irqreturn_t;

typedef irqreturn_t (* irq_handler_t)(int, void *);

struct irqaction {
    irq_handler_t    handler;
	unsigned long    flags;
	void             *dev_id;
	struct irqaction *next;
	int              irq;
	irq_handler_t    thread_fn;
	struct task_struct *thread;
	unsigned long    thread_flags;
	unsigned long    thread_mask;
	const char       *name;
	struct proc_dir_entry *dir;
};

extern irqreturn_t no_action(int cpl, void *dev_id);

extern void disable_irq_nosync(unsigned int irq);
extern void disable_irq(unsigned int irq);
extern void enable_irq(unsigned int irq);

#define force_irqthreads (0)


extern int early_irq_init(void);
extern int arch_early_irq_init(void);

#endif
