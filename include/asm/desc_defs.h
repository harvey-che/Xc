#ifndef _ASM_X86_DESC_DEFS_H
#define _ASM_X86_DESC_DEFS_H


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
