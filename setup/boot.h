#ifndef BOOT_BOOT_H
#define BOOT_BOOT_H

#include <Xc/types.h>
#include <asm/e820.h>
#include <asm/processor-flags.h>
#include <stddef.h>
#include <Xc/const.h>
#include <asm/bootparam.h>

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(*(x)))

extern struct boot_params boot_params;

#define cpu_relax() asm volatile("rep; nop")

struct biosregs {
    union {
        struct {
            u32 edi;
			u32 esi;
			u32 ebp;
			u32 _esp;
			u32 ebx;
			u32 edx;
			u32 ecx;
			u32 eax;
			u32 _fsgs;
			u32 _dses;
			u32 eflags;
		};
		struct {
            u16 di, hdi;
			u16 si, hsi;
			u16 bp, hbp;
			u16 _sp, _hsp;
			u16 bx, hbx;
			u16 dx, hdx;
			u16 cx, hcx;
			u16 ax, hax;
			u16 gs, fs;
			u16 es, ds;
			u16 flags, hflags;
		};
		struct {
            u8 dil, dih, edi2, edi3;
			u8 sil, sih, esi2, esi3;
            u8 bpl, bph, ebp2, ebp3;
            u8 _spl, _sph, _esp2, _esp3;
			u8 bl, bh, ebx2, ebx3;
			u8 dl, dh, edx2, edx3;
			u8 cl, ch, ecx2, ecx3;
			u8 al, ah, eax2, eax3;
        };
	};
};

void intcall(u8 int_no, const struct biosregs *ireg, struct biosregs *oreg);

void __attribute__((noreturn)) go_to_protected_mode(void);

void __attribute__((noreturn)) protected_mode_jump(u32 code32_start);

/* copy.S */
void *memset(void *dst, int c, size_t len);

int enable_a20(void);

void puts(const char *str);
void putchar(int ch);

static inline void outb(u8 v, u16 port)
{
    asm volatile("outb %0, %1": :"a"(v), "dN"(port));
}

static inline u8 inb(u16 port)
{
    u8 v;
	asm volatile("inb %1, %0": "=a"(v): "dN"(port));
	return v;
}

static inline void outw(u16 v, u16 port)
{
    asm volatile("outw %0, %1": :"a"(v), "dN"(port));
}

static inline u8 inw(u16 port)
{
    u16 v;
	asm volatile("inw %1, %0": "=a"(v): "dN"(port));
	return v;
}

static inline void outl(u32 v, u16 port)
{
    asm volatile("outl %0, %1": :"a"(v), "dN"(port));
}

static inline u32 inl(u32 port)
{
    u32 v;
	asm volatile("inl %1, %0": "=a"(v): "dN"(port));
	return v;
}

static inline void io_delay(void)
{
    const u16 DELAY_PORT = 0x80;
	asm volatile("outb %%al, %0": :"dN"(DELAY_PORT));
}

void initregs(struct biosregs *req);
int detect_memory_e820(void);

static inline u16 ds(void)
{
    u16 seg;
	asm("movw %%ds, %0": "=rm"(seg));
	return seg;
}

static inline void set_fs(u16 seg)
{
    asm volatile("movw %0, %%fs": :"rm"(seg));
}


static inline u16 fs(void)
{
    u16 seg;
	asm volatile("movw %%fs, %0": "=rm"(seg));
	return seg;
}

static inline void set_gs(u16 seg)
{
    asm volatile("movw %0, %%gs": :"rm"(seg));
}

static inline u16 gs(void)
{
    u16 seg;
	asm volatile("movw %%gs, %0": "=rm"(seg));
	return seg;
}

typedef unsigned int addr_t;

static inline u8 rdfs8(addr_t addr)
{
    u8 y;
	asm volatile("movb %%fs:%1, %0": "=q"(y): "m"(*(u8*)addr));
	return y;
}

static inline u16 rdfs16(addr_t addr)
{
    u16 y;
	asm volatile("movw %%fs:%1, %0": "=r"(y): "m"(*(u16*)addr));
	return y;
}

static inline u32 rdfs32(addr_t addr)
{
    u32 y;
	asm volatile("movl %%fs:%1, %0": "=r"(y): "m"(*(u32*)addr));
	return y;
}

static inline void wrfs8(u8 y, addr_t addr)
{
    asm volatile("movb %1, %%fs:%0": "+m"(*(u8*)addr): "qi"(y));
}

static inline void wrfs16(u16 y, addr_t addr)
{
    asm volatile("movw %1, %%fs:%0": "+m"(*(u16*)addr): "ri"(y));
}

static inline void wrfs32(u32 y, addr_t addr)
{
    asm volatile("movl %1, %%fs:%0": "+m"(*(u16*)addr): "ri"(y));
}

static inline u8 rdgs8(addr_t addr)
{
    u8 y;
	asm volatile("movb %%gs:%1, %0": "=q"(y): "m"(*(u8*)addr));
	return y;
}

static inline u16 rdgs16(addr_t addr)
{
    u16 y;
	asm volatile("movw %%gs:%1, %0": "=r"(y): "m"(*(u16*)addr));
	return y;
}

static inline u32 rdgs32(addr_t addr)
{
    u32 y;
	asm volatile("movl %%gs:%1, %0": "=r"(y): "m"(*(u32*)addr));
	return y;
}

static inline void wrgs8(u8 y, addr_t addr)
{
    asm volatile("movb %1, %%gs:%0": "+m"(*(u8*)addr): "qi"(y));
}

static inline void wrgs16(u16 y, addr_t addr)
{
    asm volatile("movw %1, %%gs:%0": "+m"(*(u16*)addr): "ri"(y));
}

static inline void wrgs32(u32 y, addr_t addr)
{
    asm volatile("movl %1, %%gs:%0": "+m"(*(u16*)addr): "ri"(y));
}

static inline int memcmp(const void *s1, const void *s2, size_t len)
{
    u8 diff;
	asm("repe; cmpsb; setnz %0"
			: "=qm"(diff), "+D"(s1), "+S"(s2), "+c"(len));
	return diff;
}

static inline int memcmp_fs(const void *s1, addr_t s2, size_t len)
{
    u8 diff;
	asm volatile("fs; repe; cmpsb; setnz %0"
			: "=qm"(diff), "+D"(s1), "+S"(s2), "+c"(len));
	return diff;
}

static inline int memcmp_gs(const void *s1, addr_t s2, size_t len)
{
    u8 diff;
	asm volatile("gs; repe; cmpsb; setnz %0"
			: "=qm"(diff), "+D"(s1), "+S"(s2), "+c"(len));
	return diff;
}

#endif
