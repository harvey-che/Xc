#ifndef _ASM_X86_PGTABLE_DEFS_H
#define _ASM_X86_PGTABLE_DEFS_H

#define PGDIR_SHIFT 22
#define PTRS_PER_PGD 1024

#define PUD_SHIFT 22
#define PTRS_PER_PUD 1

#define PMD_SHIFT 22
#define PTRS_PER_PMD 1

#define PAGE_SHIFT 12
#define PTRS_PER_PTE 1024

#define PTE_PFN_MASK 0xfffff000

#define PAGE_SIZE 4096

#define THREAD_SIZE (PAGE_SIZE << 2)

#define PAGE_OFFSET 0xc0000000

#define _PAGE_PRESENT   0x00000001
#define _PAGE_RW        0x00000002
#define _PAGE_USER      0x00000004
#define _PAGE_ACCESSED  0x00000020
#define _PAGE_DIRTY     0x00000040
#define _PAGE_GLOBAL    0x00000100

#define PTE_IDENT_ATTR 0x003    /* PRESENT + RW*/
#define PDE_IDENT_ATTR 0x067    /* PRESENT + RW +USER + DIRTY + ACCESSED */
#define PGD_IDENT_ATTR 0x001    /* PRESENT (no other attributes) */

#define _PAGE_TABLE (_PAGE_PRESENT | _PAGE_RW | _PAGE_USER |    \
                	 _PAGE_ACCESSED | _PAGE_DIRTY)

#define __PAGE_KERNEL_EXEC    (_PAGE_PRESENT | _PAGE_RW | _PAGE_DIRTY | _PAGE_ACCESSED |   \
		                       _PAGE_GLOBAL)

#endif
