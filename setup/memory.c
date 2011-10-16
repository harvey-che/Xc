#include "boot.h"

#define SMAP 0x534d4150

int detect_memory_e820(void)
{
    int count = 0, i = 0, j = 0;
	u32 total_physical_memsize = 0;
	struct biosregs ireg, oreg;
	struct e820entry *desc = e820_map;
    static struct e820entry buf;

    j++;
	initregs(&ireg);
	ireg.ax = 0xe820;
    ireg.cx = sizeof buf;
	ireg.edx = SMAP;
	ireg.di = (size_t)&buf;    // I wonder if this size_t casting is necessary

	do {
        intcall(0x15, &ireg, &oreg);
		ireg.ebx = oreg.ebx;
        
		if (oreg.eflags & X86_EFLAGS_CF)
			break;

		if (oreg.eax != SMAP) {
            count = 0;
			break;
		}

		*desc++ = buf;
		count++;
	} while (ireg.ebx && count < ARRAY_SIZE(e820_map));

    for (; i < count; ++i) {
        if (e820_map[i].type == AddressRangeMemory
				&& (u32)e820_map[i].addr + (u32)e820_map[i].size > total_physical_memsize) {
            total_physical_memsize = (u32)e820_map[i].addr + (u32)e820_map[i].size;
		}
	}
   
	set_fs(0x9000);
	wrfs32(total_physical_memsize, 0);

	return count;
}

