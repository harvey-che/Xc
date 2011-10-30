#ifndef _ASM_X86_PAGE_H
#define _ASM_X86_PAGE_H

#include <Xc/types.h>
#include <Xc/const.h>
#include <asm/page_types.h>
#include <Xc/string.h>

/* 
 * asm/page.h -> asm/page_types.h
 *            -> asm/page_32.h -> asm/page_32_types.h
 *            -> asm-generic/memory_model.h 
 * 
 */ 

/* 0UL if CONFIG_FLATMEM is defined */
#define ARCH_PFN_OFFSET   (0UL)

#define __pfn_to_page(pfn)    (mem_map + ((pfn) - ARCH_PFN_OFFSET))
#define __page_to_pfn(page)   ((unsigned long)((page) - mem_map) + ARCH_PFN_OFFSET)

#define page_to_pfn __page_to_pfn
#define pfn_to_page __pfn_to_page

#define virt_to_page(kaddr) pfn_to_page(__pa(kaddr) >> PAGE_SHIFT)


#define __va(x) ((void*)((unsigned long)(x) + PAGE_OFFSET))
#define __pa(x) ((unsigned long)(x) - PAGE_OFFSET)

static inline phys_addr_t virt_to_phys(volatile void *address)
{
    return __pa(address);
}

static inline void *phys_to_virt(unsigned long address)
{
    return __va(address);
}

static inline void clear_page(void *page)
{
    memset(page, 0, PAGE_SIZE);
}

#endif
