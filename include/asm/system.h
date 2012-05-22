#ifndef _ASM_X86_SYSTEM_H
#define _ASM_X86_SYSTEM_H

#ifdef CONFIG_CC_STACKPROTECTOR

#else

#define __switch_canary
#define __switch_canary_oparam
#define __switch_canary_iparam

#endif

#define switch_to(prev, next, last)    \
	do {                               \
		    unsigned long ebx, ecx, edx, esi, edi;    \
		    asm volatile("pushfl\n\t"                 \
					     "pushl %%ebp\n\t"            \
						 "movl %%esp, %[prev_sp]\n\t" \
						 "movl %[next_sp], %%esp\n\t" \
						 "movl $1f, %[prev_ip]\n\t"   \
						 "pushl %[next_ip]\n\t"       \
						 __switch_canary              \
						 "jmp __switch_to\n\t"        \
						 "1:\n\t"                     \
						 "popl %%esp\n\t"             \
						 "popfl\n\t"                  \
						 : [prev_sp] "=m" (prev->thread.sp),    \
						   [prev_ip] "=m" (prev->thread.ip),    \
						   "=a" (last),                         \
                                                                \
						   "=b" (ebx), "=c" (ecx), "=d" (edx),  \
						   "=S" (esi), "=D" (edi)               \
                                                                \
						   __switch_canary_oparam               \
                                                                \
						 : [next_sp] "m" (next->thread.sp),     \
						   [next_ip] "m" (next->thread.ip),     \
                                                                \
						   [prev] "a" (prev),                   \
						   [next] "d" (next)                    \
						                                        \
						   __switch_canary_iparam               \
						                                        \
						 : "memory");                           \
	} while(0)


#endif
