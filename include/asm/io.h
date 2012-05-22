#ifndef _ASM_X86_IO_H
#define _ASM_X86_IO_H
#include <Xc/types.h>
#include <asm/pagetable.h>

static inline phys_addr_t virt_to_phys(volatile void *address)
{
    return __pa(address);
}

static inline void *phys_to_virt(phys_addr_t address)
{
    return __va(address);
}

#define BUILDIO(bwl, bw, type)    \
static inline void out##bwl(unsigned type value, int port)    \
{                                                             \
    asm volatile("out" #bwl " %" #bw "0, %w1"                 \
	             : : "a"(value), "Nd"(port));                 \
}                                                             \
                                                              \
static inline unsigned type in##bwl(int port)                 \
{                                                             \
    unsigned type value;                                      \
	asm volatile("in" #bwl " %w1, %" #bw "0"                  \
			     : "=a"(value) : "Nd"(port));                 \
	return value;                                             \
}

BUILDIO(b, b, char)
BUILDIO(w, w, short)
BUILDIO(l, , int)

#endif
