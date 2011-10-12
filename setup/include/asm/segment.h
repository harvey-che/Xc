#ifndef _ASM_X86_SEGMENT_H
#define _ASM_X86_SEGMENT_H

#define GDT_ENTRY_BOOT_CS    2
#define __BOOT_CS    (GDT_ENTRY_BOOT_CS * 8)

#define GDT_ENTRY_BOOT_DS    (GDT_ENTRY_BOOT_CS + 1)
#define __BOOT_DS    (GDT_ENTRY_BOOT_DS * 8)

#define GDT_ENTRY_BOOT_TSS   (GDT_ENTRY_BOOT_CS + 2)
#define __BOOT_TSS    (GDT_ENTRY_BOOT_TSS * 8)

#endif
