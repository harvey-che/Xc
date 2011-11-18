#include <Xc/kernel.h>
#include <asm/desc.h>
#include <asm/cache.h>
#include <asm/irq_vectors.h>
#include <Xc/init.h>
#include <Xc/bitops.h>
#include <asm/x86_init.h>

gate_desc idt_table[NR_VECTORS] __attribute__((aligned(L1_CACHE_BYTES))) = { { { { 0, 0 } } }, };

DECLARE_BITMAP(used_vectors, NR_VECTORS);

void __init trap_init(void)
{
    int i;
    
	/*
	set_intr_gate(0, &divide_error);
	set_intr_gate_ist(2, &nmi, NMI_STACK);
	set_system_intr_gate(4, &overflow);
	set_intr_gate(5, &bounds);
	set_intr_gate(6, &invalid_op);
	set_intr_gate(7, &device_not_available);

	set_intr_gate(8, GDT_ENTRY_DOUBLEFAULT_TSS);

	set_intr_gate(9, &coprocessor_segment_overrun);
	set_intr_gate(10, &invalid_TSS);
	set_intr_gate(11, &segment_not_present);
	set_intr_gate_ist(12, &stack_segment, STACKFAULT_STACK);
	set_intr_gate(13, &general_protection);
	set_intr_gate(15, &spurious_interrupt_bug);
	set_intr_gate(16, &coprocessor_error);
	set_intr_gate(17, &alignment_check);

	set_intr_gate(19, &simd_coprocessor_error);
    */
	for (i = 0; i < FIRST_EXTERNAL_VECTOR; i++)
		set_bit(i, used_vectors);

	/* set_system_trap_gate(SYSCALL_VECTOR, &system_call); */
	set_bit(SYSCALL_VECTOR, used_vectors);

	/* cpu_init(); */
	x86_init.irqs.trap_init();
}
