#include "boot.h"
#include <asm/segment.h>

extern u32 code32_start;

struct gdt_ptr {
    u16 len;
	u32 ptr;
} __attribute__((packed));

static void setup_gdt(void)
{
    static const u64 boot_gdt[] __attribute__((aligned(16))) = {
		/*
		[GDT_ENTRY_BOOT_CS] = 0x00cf9b000000ffffULL,
		[GDT_ENTRY_BOOT_DS] = 0x00cf93000000ffffULL,
		[GDT_ENTRY_BOOT_TSS] = 0x0000890010000067ULL
		*/
		
        [GDT_ENTRY_BOOT_CS] = GDT_ENTRY(0xc09b, 0, 0xfffff),
		[GDT_ENTRY_BOOT_DS] = GDT_ENTRY(0xc093, 0, 0xfffff),
		[GDT_ENTRY_BOOT_TSS] = GDT_ENTRY(0x0089, 4096, 103)
		
	};

	static struct gdt_ptr gdt;

	gdt.len = sizeof(boot_gdt) - 1;
	gdt.ptr = (u32) &boot_gdt + (ds() << 4);

	asm volatile("lgdtl %0": :"m"(gdt));
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
