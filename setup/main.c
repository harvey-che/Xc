#include "boot.h"

struct e820entry e820_map[E820MAX];

u32 code32_start = 0x100000;

void prepare_for_pm(void)
{
	/* Clear bootsect */
	int i = 0;
    set_fs(0x9000);
	for (; i < 128; ++i)
	{
        wrfs32(0, i << 2);
	}

    detect_memory_e820();
    go_to_protected_mode();
}
	
