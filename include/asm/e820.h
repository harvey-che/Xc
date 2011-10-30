#ifndef _ASM_X86_E820_H
#define _ASM_X86_E820_H

#include <Xc/types.h>
#include <asm/processor-flags.h>

#define E820MAX 128

#define E820_RAM        1
#define E820_RESERVED   2
#define E820_ACPI       3
#define E820_NVS        4
#define E820_UNUSABLE   5

#define E820_RESERVED_KERN   128

#ifndef __ASSEMBLY__

struct e820entry {
    __u64 addr;     /* start of memory segment */
	__u64 size;     /* size of memory segment */
	__u32 type;     /* type of memory segment */
} __attribute__((packed));


struct e820map {
    __u32 nr_map;
	struct e820entry map[E820MAX];
};

extern struct e820map e820;

int sanitize_e820_map(struct e820entry *biosmap, int max_nr_map, u32 *pnr_map);
void memblock_x86_fill(void);

#endif

#endif
