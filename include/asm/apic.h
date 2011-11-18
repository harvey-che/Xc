#ifndef _ASM_X86_APIC_H
#define _ASM_X86_APIC_H

/* asm/apicdef.h */
#define APIC_EOI 0xB0
/* end -- asm/apicdef.h */

static inline void apic_write(u32 reg, u32 val) {}

static inline void ack_APIC_irq(void)
{
    apic_write(APIC_EOI, 0);
}

#endif
