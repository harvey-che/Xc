#ifndef _ASM_X86_DESC_H
#define _ASM_X86_DESC_H

#include <Xc/types.h>
#include <asm/segment.h>
#include <Xc/string.h>
#include <asm/desc_defs.h>
#include <Xc/percpu.h>
#include <Xc/compiler-gcc.h>
#include <stddef.h>
#include <asm/processor.h>
#include <Xc/mm_types.h>
#include <Xc/smp.h>

/* asm/desc_defs.h */
#if 0
enum {
    GATE_INTERRUPT = 0xE,
	GATE_TRAP = 0xF,
	GATE_CALL = 0xC,
	GATE_TASK = 0x5,
};

enum {
    DESC_TSS = 0x9,
	DESC_LDT = 0x2,
	DESCTYPE_S = 0x10,
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

#endif

/* end -- asm/desc_defs.h */

/* asm/ldt.h */

#define LDT_ENTRIES    8192
#define LDT_ENTRY_SIZE 0

/* end -- asm/ldt.h */

static inline void pack_gate(gate_desc *gate, unsigned char type, unsigned long base, unsigned dpl, unsigned flags, unsigned short seg)
{
    gate->a = (seg << 16) | (base & 0xffff);
	gate->b = (base & 0xffff0000) | (((0x80 | type | (dpl << 5)) & 0xff) << 8);
}

extern struct desc_ptr idt_descr;

extern gate_desc idt_table[];

struct gdt_page {
    struct desc_struct gdt[GDT_ENTRIES];
} __attribute__((aligned(PAGE_SIZE)));

DECLARE_PER_CPU_PAGE_ALIGNED(struct gdt_page, gdt_page);

static inline struct desc_struct *get_cpu_gdt_table(unsigned int cpu)
{
    return per_cpu(gdt_page, cpu).gdt;
}

#define GDT_ENTRY_INIT(flags, base, limit) { { {    \
	.a = ((limit) & 0xffff) | (((base) & 0xffff) << 16), \
	.b = (((base) & 0xff0000) >> 16) | (((flags) & 0xf0ff) << 8) | \
	     ((limit) & 0xf0000) | ((base) & 0xff000000), }}}

#ifdef CONFIG_PARAVIRT

#else

#define load_TR_desc() native_load_tr_desc()
#define load_idt(dtr) native_load_idt(dtr)
#define load_ldt(ldt) asm volatile("lldt %0": : "m" (ldt))

#define set_ldt native_set_ldt

#define write_idt_entry(dt, entry, g)    native_write_idt_entry(dt, entry, g)
#define write_gdt_entry(dt, entry, desc, type) native_write_gdt_entry(dt, entry, desc, type)

#endif

static inline void native_write_idt_entry(gate_desc *idt, int entry, const gate_desc *gate)
{
    memcpy(&idt[entry], gate, sizeof(*gate));
}

static inline void native_write_gdt_entry(struct desc_struct *gdt, int entry, const void *desc, int type)
{
    unsigned int size;

	switch(type) {
    case DESC_TSS: size = sizeof(tss_desc); break;
	case DESC_LDT: size = sizeof(ldt_desc); break;
	default: size = sizeof(*gdt);
	}

	memcpy(&gdt[entry], desc, size);
}

static inline void pack_descriptor(struct desc_struct *desc, unsigned long base, 
		                           unsigned long limit, unsigned char type, 
								   unsigned char flags)
{
    desc->a = ((base & 0xffff) << 16) | (limit & 0xffff);
	desc->b = (base & 0xff000000) | ((base & 0xff0000) >> 16) | (limit & 0x000f0000)
		       | ((type & 0xff) << 8) | ((flags & 0xf) << 20);
	desc->p = 1;
}

static inline void set_tssldt_descriptor(void *d, unsigned long addr, unsigned type, unsigned size)
{
    pack_descriptor((struct desc_struct *)d, addr, size, 0x80 | type, 0);
}

static inline void __set_tss_desc(unsigned cpu, unsigned int entry, void *addr)
{
    struct desc_struct *d = get_cpu_gdt_table(cpu);
	tss_desc tss;

	set_tssldt_descriptor(&tss, (unsigned long)addr, DESC_TSS, IO_BITMAP_OFFSET + 
			              IO_BITMAP_BYTES + sizeof(unsigned long) - 1);
	write_gdt_entry(d, entry, &tss, DESC_TSS);
}

#define set_tss_desc(cpu, addr) __set_tss_desc(cpu, GDT_ENTRY_TSS, addr)

static inline void native_load_idt(const struct desc_ptr *dtr)
{
	asm volatile("lidt %0": : "m" (*dtr));
}

static inline void native_set_ldt(const void *addr, unsigned int entries)
{
    if (entries == 0)
		asm volatile("lldt %w0": : "q" (0));
	else {
        unsigned cpu = smp_processor_id();
		ldt_desc ldt;

		set_tssldt_descriptor(&ldt, (unsigned long)addr, DESC_LDT, entries * LDT_ENTRY_SIZE - 1);
		write_gdt_entry(get_cpu_gdt_table(cpu), GDT_ENTRY_LDT, &ldt, DESC_LDT);
		asm volatile("lldt %w0": : "q" (GDT_ENTRY_LDT*8));
	}
}

static inline void native_load_tr_desc(void)
{
    asm volatile("ltr %w0": : "q" (GDT_ENTRY_TSS * 8));
}


extern unsigned long used_vectors[];

static inline void load_LDT_nolock(mm_context_t *pc)
{
    set_ldt(pc->ldt, pc->size);
}

static inline void load_LDT(mm_context_t *pc)
{
    preempt_disable();
	load_LDT_nolock(pc);
	preempt_enable();
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

#endif
