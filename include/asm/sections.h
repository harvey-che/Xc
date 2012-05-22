#ifndef _ASM_SECTIONS_H
#define _ASM_SECTIONS_H

extern char _text[], _etext[];
extern char _data[], _edata[];
extern char __bss_start[], __bss_stop[];
extern char _end[];
extern char __brk_base[];

#endif
