#ifndef _ASM_X86_PAGE_TYPES_H
#define _ASM_X86_PAGE_TYPES_H

#include <Xc/const.h>

#define PAGE_SHIFT 12
#define PAGE_SIZE  (_AC(1, UL) << PAGE_SHIFT)
#define PAGE_MASK  (~(PAGE_SIZE - 1))

#define __PAGE_OFFSET _AC(0xc0000000, UL)
#define PAGE_OFFSET   ((unsigned long)__PAGE_OFFSET)

#define THREAD_ORDER 1
#define THREAD_SIZE (PAGE_SIZE << THREAD_ORDER)

#ifndef __ASSEMBLY__
extern void initmem_init(void);
extern void free_initmem(void);
#endif

#endif
