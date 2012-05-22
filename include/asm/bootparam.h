#ifndef _ASM_X86_BOOTPARAM_H
#define _ASM_X86_BOOTPARAM_H

#include <asm/e820.h>

struct boot_params {
	__u8 e820_entries;
	__u8 pad0[3];
    struct e820entry e820_map[E820MAX]; /* 128 * 20 */
	__u8 pad1[4096- 2564];
} __attribute__((packed));


#endif
