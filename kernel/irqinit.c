#include <Xc/init.h>
#include <Xc/bitops.h>
#include <asm/irq_vectors.h>
#include <asm/desc.h>
#include <asm/io.h>
#include <Xc/spinlock.h>
#include <asm/i8259.h>
#include <asm/hw_irq.h>
#include <Xc/percpu.h>
#include <Xc/irq.h>
#include <asm/x86_init.h>
#include <asm/desc.h>
#include <asm/acpi.h>
#include <Xc/smp.h>

static struct irqaction irq2 = {
    .handler = no_action,
	.name = "cascade",
	.flags = IRQF_NO_THREAD,
};

DEFINE_PER_CPU(vector_irq_t, vector_irq) = {
    [0 ... NR_VECTORS - 1] = -1,
};

void __init init_ISA_irqs(void)
{
    struct irq_chip *chip = legacy_pic->chip;
	const char *name = chip->name;
	int i;

	/* init_8259A() initialize the 8259A chip */
	legacy_pic->init(0);
	
	for (i = 0; i < legacy_pic->nr_legacy_irqs; i++)
		irq_set_chip_and_handler_name(i, chip, handle_level_irq, name);
}

void __init init_IRQ(void)
{
    int i;

	/* x86_add_irq_domains(); */
	for (i = 0; i < legacy_pic->nr_legacy_irqs; i++)
		per_cpu(vector_irq, 0)[IRQ0_VECTOR + i] = i;

	x86_init.irqs.intr_init();
}

static void __init smp_intr_init(void)
{
}

static void __init apic_intr_init(void)
{
    smp_intr_init();
}

void __init native_init_IRQ(void)
{
	int i;
    x86_init.irqs.pre_vector_init();

	/* apic_intr_init(); */

	for (i = FIRST_EXTERNAL_VECTOR; i < NR_VECTORS; i++) {
        if (!test_bit(i, used_vectors))
			set_intr_gate(i, interrupt[i - FIRST_EXTERNAL_VECTOR]);
	}

	if (!acpi_ioapic && !of_ioapic)
		setup_irq(2, &irq2);
    /*
	if (boot_cpu_data.hard_math && !cpu_has_fpu)
		setup_irq(FPU_IRQ, &fpu_irq);
    */
	/* hardirq and softirq are not supported yet */
	irq_ctx_init(smp_processor_id());
}
