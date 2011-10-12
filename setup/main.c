#include "boot.h"

struct e820entry e820_map[E820MAX];

u32 code32_start = 0x10000;

void prepare_for_pm(void)
{
    detect_memory_e820();
    go_to_protected_mode();
}
	
