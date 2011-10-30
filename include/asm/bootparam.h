#ifndef _ASM_X86_BOOTPARAM_H
#define _ASM_X86_BOOTPARAM_H

#include <asm/e820.h>

struct boot_params {
	__u8 e820_entries;
    struct e820entry e820_map[E820MAX];
	__u8 pad[4096-sizeof(__u8) - (E820MAX * sizeof(struct e820entry))];
} __attribute__((packed));


#endif
