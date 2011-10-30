#ifndef _XC_IRQFLAGS_H
#define _XC_IRQFLAGS_H

#include <Xc/typecheck.h>
#include <asm/processor-flags.h>

/*
 * linux/irqflags.h -> asm/irqflags.h
 */

/* x86/asm/irqflags.h */

static inline unsigned long native_save_fl(void)
{
    unsigned long flags;

	/* 
	 * "=rm" is safe here, because "pop" adjusts the stack before it evaluates its effective address
	 * By harvey, I don't understand the comments above
	 */
	asm volatile ("# __raw_save_flags \n\t"
			      "pushf; pop %0"
				  : "=rm"(flags)
				  : : "memory");
	return flags;
}

static inline void native_restore_fl(unsigned long flags)
{
    asm volatile ("push %0; popf"
			      : : "g"(flags)
				  : "memory", "cc");
}

static inline void native_irq_disable(void)
{
    asm volatile ("cli": : : "memory");
}

static inline void native_irq_enable(void)
{
    asm volatile ("sti": : : "memory");
}

static inline unsigned long arch_local_save_flags(void)
{
    return native_save_fl();
}

static inline void arch_local_irq_disable(void)
{
    native_irq_disable();
}

static inline void arch_local_irq_enable(void)
{
    native_irq_enable();
}

static inline unsigned long arch_local_irq_save(void)
{
    unsigned long flags = arch_local_save_flags();
	arch_local_irq_disable();
	return flags;
}

static inline void arch_local_irq_restore(unsigned long flags)
{
    native_restore_fl(flags);
}

static inline int arch_irqs_disabled_flags(unsigned long flags)
{
    return !(flags & X86_EFLAGS_IF);
}

static inline int arch_irqs_disabled(void)
{
    unsigned long flags = arch_local_save_flags();
	return arch_irqs_disabled_flags(flags);
}

/* end -- x86/asm/irqflags.h  */

#define raw_local_irq_save(flags)    \
	do {                             \
		    typecheck(unsigned long , flags);   \
		    flags = arch_local_irq_save();      \
	} while (0)

#define raw_local_irq_restore(flags)    \
	do {                                \
		typecheck(unsigned long, flags);    \
		arch_local_irq_restore(flags);  \
	} while (0)

#define local_irq_save(flags)         \
	do {                              \
		raw_local_irq_save(flags);    \
	} while (0)

#define local_irq_restore(flags)      \
	do {                              \
		raw_local_irq_restore(flags); \
	} while (0)

#define raw_local_irq_disable() arch_local_irq_disable()
#define raw_local_irq_enable() arch_local_irq_enable()

#define raw_irqs_disabled() (arch_irqs_disabled())

#define local_irq_enable() do { raw_local_irq_enable(); } while (0)
#define local_irq_disable() do { raw_local_irq_disable(); } while (0)

#define irqs_disabled() (raw_irqs_disabled())

#endif
