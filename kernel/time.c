#include <Xc/init.h>
#include <Xc/irq.h>
#include <asm/x86_init.h>

/* kernel/timer.c */
#include <asm/cache.h>
#include <Xc/jiffies.h>
u64 jiffies_64 __cacheline_aligned_in_smp = INITIAL_JIFFIES;
/* end -- kernel/timer.c */

static irqreturn_t timer_interrupt(int irq, void *dev_id)
{
    /* inc_irq_stat(irq0_irqs); */

	/* global_clock_event->event_handler(global_clock_event); */

	/*
	if (MCA_bus)
		outb_p(inb_p(0x61) | 0x80, 0x61);
	*/

	return IRQ_HANDLED;
}

static struct irqaction irq0 = {
    .handler = timer_interrupt,
	.flags = IRQF_DISABLED | IRQF_NOBALANCING | IRQF_IRQPOLL | IRQF_TIMER,
	.name = "timer"
};
void __init setup_default_timer_irq(void)
{
    setup_irq(0, &irq0);
}

void __init hpet_time_init(void)
{
	/*
    if (!hpet_enable())
		setup_pit_timer();
	*/
	setup_default_timer_irq();
}

static __init void x86_late_time_init(void)
{
    x86_init.timers.timer_init();
	/* tsc_init(); */
}

void __init time_init(void)
{
    late_time_init = x86_late_time_init;
}
