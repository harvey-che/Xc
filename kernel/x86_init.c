#include <Xc/init.h>
#include <asm/x86_init.h>
#include <Xc/irq.h>
#include <Xc/time.h>

void __cpuinit x86_init_noop(void) {}

struct x86_init_ops x86_init __initdata = {
    .irqs = {
        .pre_vector_init = init_ISA_irqs,
		.intr_init = native_init_IRQ,
		.trap_init = x86_init_noop,
	},
	.timers = {
        .timer_init = hpet_time_init,
	},
};
