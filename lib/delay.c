#include <Xc/delay.h>
#include <Xc/jiffies.h>

unsigned long loops_per_jiffy = (1 << 12);

static void delay_loop(unsigned long loops)
{
    asm volatile("test %0, %0\n"
			     "jz 3f\n"
				 "jmp 1f\n"

				 ".align 16\n"
				 "1: jmp 2f\n"

				 ".align 16\n"
				 "2: dec %0\n"
				 "jnz 2b\n"
				 "3: dec %0\n"
                 :
				 : "a" (loops));
}

static void (*delay_fn)(unsigned long) = delay_loop;

void __delay(unsigned long loops)
{
    delay_fn(loops);
}

inline void __const_udelay(unsigned long xloops)
{
    int id0;

	xloops *= 4;
	asm("mull %%edx"
		: "=d" (xloops), "=&a" (id0)
		: "1" (xloops), "0" ((loops_per_jiffy) * (HZ / 4)));
	__delay(++xloops);
}
