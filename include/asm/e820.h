#ifndef _ASM_X86_E820_H
#define _ASM_X86_E820_H

#include <types.h>
#include <asm/processor-flags.h>

#define E820MAX 128

#ifndef __ASSEMBLY__

struct e820entry {
    __u64 addr;     /* start of memory segment */
	__u64 size;     /* size of memory segment */
	__u32 type;     /* type of memory segment */
} __attribute__((packed));

extern struct e820entry e820_map[E820MAX];

#endif

#define AddressRangeMemory 1
#define AddressRangeReserved 2

#endif
