#ifndef _ASM_X86_DESC_H
#define _ASM_X86_DESC_H

#include <Xc/types.h>
#include <asm/segment.h>
#include <Xc/string.h>

/* asm/desc_defs.h */

enum {
    GATE_INTERRUPT = 0xE,
	GATE_TRAP = 0xF,
	GATE_CALL = 0xC,
	GATE_TASK = 0x5,
};


struct desc_struct {
    union {
		struct {
			unsigned int a;
			unsigned int b;
		};
		struct {
            u16 limit0;
			u16 base0;
			unsigned base1:8, type:4, s:1, dpl:2, p:1;
			unsigned limit:4, avl:1, l:1, d:1, g:1, base2:8;
		};
	};
} __attribute__((packed));

#ifdef CONFIG_X86_64

#else

typedef struct desc_struct gate_desc;
typedef struct desc_struct ldt_desc;
typedef struct desc_struct tss_desc;

struct desc_ptr {
    unsigned short size;
	unsigned long address;
} __attribute__((packed));


#endif

/* end -- asm/desc_defs.h */
static inline void pack_gate(gate_desc *gate, unsigned char type, unsigned long base, unsigned dpl, unsigned flags, unsigned short seg)
{
    gate->a = (seg << 16) | (base & 0xffff);
	gate->b = (base & 0xffff0000) | (((0x80 | type | (dpl << 5)) & 0xff) << 8);
}

extern struct desc_ptr idt_descr;

extern gate_desc idt_table[];

struct gdt_page {
    struct desc_struct gdt[GDT_ENTRIES];
};

#define GDT_ENTRY_INIT(flags, base, limit) { { {    \
	.a = ((limit) & 0xffff) | (((base) & 0xffff) << 16), \
	.b = (((base) & 0xff0000) >> 16) | (((flags) & 0xf0ff) << 8) | \
	     ((limit) & 0xf0000) | ((base) & 0xff000000), }}}

#ifdef CONFIG_PARAVIRT

#else

#define write_idt_entry(dt, entry, g)    native_write_idt_entry(dt, entry, g)

#endif

static inline void native_write_idt_entry(gate_desc *idt, int entry, const gate_desc *gate)
{
    memcpy(&idt[entry], gate, sizeof(*gate));
}

static inline void _set_gate(int gate, unsigned type, void *addr, unsigned dpl, unsigned ist, unsigned seg)
{
    gate_desc s;

	pack_gate(&s, type, (unsigned long)addr, dpl, ist, seg);

	write_idt_entry(idt_table, gate, &s);
}

static inline void set_intr_gate(unsigned int n, void *addr)
{
	/* BUG_ON((unsigned)n > 0xFF); */
    _set_gate(n, GATE_INTERRUPT, addr, 0, 0, __KERNEL_CS);
}

static inline void set_intr_gate_ist(int n, void *addr, unsigned ist)
{
    _set_gate(n, GATE_INTERRUPT, addr, 0, ist, __KERNEL_CS);
}

static inline void native_load_idt(const struct desc_ptr *dtr)
{
	asm volatile("lidt %0": : "m" (*dtr));
}

#define load_idt(dtr) native_load_idt(dtr)

extern unsigned long used_vectors[];

#endif
