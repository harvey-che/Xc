#ifndef _ASM_X86_ACPI_H
#define _ASM_X86_ACPI_H

#ifdef CONFIG_ACPI

#else

#define acpi_lapic 0
#define acpi_ioapic 0


#endif

/* asm/prom.h */
#ifdef CONFIG_OF

#else

#define of_ioapic 0

#endif
/* end -- asm/prom.h */

#endif
