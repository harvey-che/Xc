#ifndef _ASM_X86_DESC_H
#define _ASM_X86_DESC_H

#include <Xc/types.h>
#include <asm/segment.h>

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
}; __attribute__((packed));

#ifdef CONFIG_X86_64

#else

typedef struct desc_struct gate_desc;
typedef struct desc_struct ldt_desc;
typedef struct desc_struct tss_desc;

#endif

extern gate_desc idt_table[];

struct gdt_page {
    struct desc_struct gdt[GDT_ENTRIES];
};

#define GDT_ENTRY_INIT(flags, base, limit) { { {    \
	.a = ((limit) & 0xffff) | (((base) & 0xffff) << 16), \
	.b = (((base) & 0xff0000) >> 16) | (((flags) & 0xf0ff) << 8) | \
	     ((limit) & 0xf0000) | ((base) & 0xff000000), }}}

#endif
