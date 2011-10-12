#ifndef _ASM_X86_BOOT_H
#define _ASM_X86_BOOT_H

#include <asm-generic/types.h>
#include <asm/processor-flags.h>

#define E820MAX 128

struct e820entry {
    __u64 addr;     /* start of memory segment */
	__u64 size;     /* size of memory segment */
	__u32 type;     /* type of memory segment */
} __attribute__((packed));

extern struct e820entry e820_map[E820MAX];

#endif
