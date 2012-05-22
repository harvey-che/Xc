#ifndef _ASM_X86_SEGMENT_H
#define _ASM_X86_SEGMENT_H

#include <Xc/const.h>


#define GDT_ENTRY_BOOT_CS    2
#define __BOOT_CS    (GDT_ENTRY_BOOT_CS * 8)

#define GDT_ENTRY_BOOT_DS    (GDT_ENTRY_BOOT_CS + 1)
#define __BOOT_DS    (GDT_ENTRY_BOOT_DS * 8)

#define GDT_ENTRY_BOOT_TSS   (GDT_ENTRY_BOOT_CS + 2)
#define __BOOT_TSS    (GDT_ENTRY_BOOT_TSS * 8)

#define GDT_ENTRY(flags, base, limit)    \
	((((base) & _AC(0xff000000, ULL)) << (56 - 24))  | \
	 (((flags) & _AC(0x0000f0ff, ULL)) << 40)  | \
	 (((limit) & _AC(0x000f0000, ULL)) << (48 - 16))  | \
	 (((base) & _AC(0x00ffffff, ULL)) << 16)  | \
	 (((limit) & _AC(0x0000ffff, ULL))))

#ifdef CONFIG_X86_32

#define GDT_ENTRY_TLS_MIN            6
#define GDT_ENTRY_TLS_MAX            (GDT_ENTRY_TLS_MIN + GDT_ENTRY_TLS_ENTRIES - 1)

#define GDT_ENTRY_DEFAULT_USER_CS    14
#define GDT_ENTRY_DEFAULT_USER_DS    15

#define GDT_ENTRY_KERNEL_BASE        (12)

#define GDT_ENTRY_KERNEL_CS          (GDT_ENTRY_KERNEL_BASE + 0)
#define GDT_ENTRY_KERNEL_DS          (GDT_ENTRY_KERNEL_BASE + 1)

#define GDT_ENTRY_TSS                (GDT_ENTRY_KERNEL_BASE + 4)
#define GDT_ENTRY_LDT                (GDT_ENTRY_KERNEL_BASE + 5)

#define GDT_ENTRY_PNPBIOS_BASE       (GDT_ENTRY_KERNEL_BASE + 6)
#define GDT_ENTRY_APMBIOS_BASE       (GDT_ENTRY_KERNEL_BASE + 11)

#define GDT_ENTRY_ESPFIX_SS          (GDT_ENTRY_KERNEL_BASE + 14)
#define __ESPFIX_SS                  (GDT_ENTRY_ESPFIX_SS * 8)

#define GDT_ENTRY_PERCPU             (GDT_ENTRY_KERNEL_BASE + 15)

/*  */
#ifdef CONFIG_SMP

#else

#define __KERNEL_PERCPU 0

#endif

#define GDT_ENTRY_STACK_CANARY       (GDT_ENTRY_KERNEL_BASE + 16)

/*  */
#ifdef CONFIG_CC_STACK_CANARY

#else

#define __KERNEL_STACK_CANARY        0
#define GDT_STACK_CANARY_INIT

#endif

#define GDT_ENTRY_DOUBLEFAULT_TSS    31

#define GDT_ENTRIES 32

#define GDT_ENTRY_PNPBIOS_CS32    (GDT_ENTRY_PNPBIOS_BASE + 0)
#define GDT_ENTRY_PNPBIOS_CS16    (GDT_ENTRY_PNPBIOS_BASE + 1)
#define GDT_ENTRY_PNPBIOS_DS      (GDT_ENTRY_PNPBIOS_BASE + 2)
#define GDT_ENTRY_PNPBIOS_TS1     (GDT_ENTRY_PNPBIOS_BASE + 3)
#define GDT_ENTRY_PNPBIOS_TS2     (GDT_ENTRY_PNPBIOS_BASE + 4)

#define SEGMENT_RPL_MASK    0x3
#define SEGMENT_TI_MASK     0x4

#define USER_RPL            0x3

#define SEGMENT_LDT         0x4
#define SEGMENT_GDT         0x0

#else /* CONFIG_X86_32 */

#endif /* CONFIG_X86_32 */

#define __KERNEL_CS   (GDT_ENTRY_KERNEL_CS * 8)
#define __KERNEL_DS   (GDT_ENTRY_KERNEL_DS * 8)
#define __USER_DS     (GDT_ENTRY_DEFAULT_USER_DS * 8 + 3)
#define __USER_CS     (GDT_ENTRY_DEFAULT_USER_CS * 8 + 3)

#define IDT_ENTRIES 256
#define GDT_ENTRY_TLS_ENTRIES 3
#define TLS_SIZE (GDT_ENTRY_TLS_ENTRIES * 8)

#endif
