#ifndef _XC_IRQDESC_H
#define _XC_IRQDESC_H

struct timer_rand_state;

struct irq_desc {
    struct irq_data    irq_data;
	struct timer_rand_state *timer_rand_state;
	unsigned int       *kstat_irqs;
	irq_flow_handler_t handle_irq;

	struct irqaction   *action;
	unsigned int       status_use_accessors;
	unsigned int       core_internal_state_do_not_mess_with_it;
	unsigned int       depth;
	unsigned int       wake_depth;
	unsigned int       irq_count;
	unsigned long      last_unhandled;
	unsigned int       irqs_unhandled;
	raw_spinlock_t     lock;

	cpumask_var_t      pending_mask;

    unsigned long      threads_oneshot;
	atomic_t           threads_active;
	wait_queue_head_t  wait_for_threads;

	struct module      *owner;
	const char         *name;
};

#endif
