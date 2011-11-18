#ifndef _ASM_X86_PAGE_TYPES_H
#define _ASM_X86_PAGE_TYPES_H

#include <Xc/const.h>
#include <Xc/types.h>

/*
 *  asm/page_types.h -> asm/page_32_types.h
 */
#define PAGE_SHIFT 12
#define PAGE_SIZE  (_AC(1, UL) << PAGE_SHIFT)
#define PAGE_MASK  (~(PAGE_SIZE - 1))

#define __PAGE_OFFSET _AC(0xc0000000, UL)

#define THREAD_ORDER 1
#define THREAD_SIZE (PAGE_SIZE << THREAD_ORDER)

#ifndef __ASSEMBLY__

#define PAGE_OFFSET   ((unsigned long)__PAGE_OFFSET)

extern void initmem_init(void);
extern void free_initmem(void);

extern unsigned long max_low_pfn_mapped;
extern unsigned long max_pfn_mapped;

extern void find_low_pfn_range(void);

static inline inline phys_addr_t get_max_mapped(void)
{
    return (phys_addr_t)max_pfn_mapped << PAGE_SHIFT;
}

extern unsigned long init_memory_mapping(unsigned long start, unsigned long end);

#endif

#endif
