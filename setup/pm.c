#include "boot.h"

extern u32 code32_start;
static void setup_gdt(void)
{

}

static void setup_idt(void)
{

}

static void mask_all_interrupts(void)
{

}

void go_to_protected_mode(void)
{
    enable_a20();

    setup_idt();
	setup_gdt();

	protected_mode_jump(code32_start);
}
