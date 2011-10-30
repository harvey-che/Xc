#ifndef _ASM_X86_IO_H
#define _ASM_X86_IO_H
#include <types.h>
#include <asm/pagetable.h>

static inline phys_addr_t virt_to_phys(volatile void *address)
{
    return __pa(address);
}

static inline void *phys_to_virt(phys_addr_t address)
{
    return __va(address);
}

#endif
