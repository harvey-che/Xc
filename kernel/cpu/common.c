#include <Xc/init.h>
#include <asm/desc.h>

struct gdt_page gdt_page = { .gdt = {
    [GDT_ENTRY_KERNEL_CS]       = GDT_ENTRY_INIT(0xc09a, 0, 0xfffff),
	[GDT_ENTRY_KERNEL_DS]       = GDT_ENTRY_INIT(0xc092, 0, 0xfffff),
	[GDT_ENTRY_DEFAULT_USER_CS] = GDT_ENTRY_INIT(0xc0fa, 0, 0xfffff),
	[GDT_ENTRY_DEFAULT_USER_DS] = GDT_ENTRY_INIT(0xc0f2, 0, 0xfffff),

	[GDT_ENTRY_PNPBIOS_CS32]    = GDT_ENTRY_INIT(0x409a, 0, 0xffff),
	[GDT_ENTRY_PNPBIOS_CS16]    = GDT_ENTRY_INIT(0x009a, 0, 0xffff),
	[GDT_ENTRY_PNPBIOS_DS]      = GDT_ENTRY_INIT(0x0092, 0, 0xffff),
	[GDT_ENTRY_PNPBIOS_TS1]     = GDT_ENTRY_INIT(0x0092, 0, 0),
	[GDT_ENTRY_PNPBIOS_TS2]     = GDT_ENTRY_INIT(0x0092, 0, 0),

	[GDT_ENTRY_APMBIOS_BASE]       = GDT_ENTRY_INIT(0x409a, 0, 0xffff),
	[GDT_ENTRY_APMBIOS_BASE + 1]   = GDT_ENTRY_INIT(0x009a, 0, 0xffff),
	[GDT_ENTRY_APMBIOS_BASE + 2]   = GDT_ENTRY_INIT(0x4092, 0, 0xffff),
	[GDT_ENTRY_ESPFIX_SS]          = GDT_ENTRY_INIT(0xc092, 0, 0xfffff),
	[GDT_ENTRY_PERCPU]             = GDT_ENTRY_INIT(0xc092, 0, 0xfffff),
	GDT_STACK_CANARY_INIT
} };


void __cpuinit cpu_init(void)
{
    load_idt((const struct desc_ptr *)&idt_descr);
}
