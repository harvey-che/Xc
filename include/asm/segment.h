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

#define GDT_ENTRY_KERNEL_BASE    12
#define GDT_ENTRY_KERNEL_CS      (GDT_ENTRY_KERNEL_BASE + 0)
#define GDT_ENTRY_KERNEL_DS      (GDT_ENTRY_KERNEL_BASE + 1)
#define GDT_ENTRY_KERNEL_TSS     (GDT_ENTRY_KERNEL_BASE + 4)
#endif
