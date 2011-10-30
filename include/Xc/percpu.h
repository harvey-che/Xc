#ifndef _XC_PERCPU_DEFS_H
#define _XC_PERCPU_DEFS_H

/* 
 * CONFIG_SMP is not defined
 * linux/percpu.h -> asm/percpu.h -> asm-generic/percpu.h -> percpu-def.h
 */

/* Don't support SMP */
#define PER_CPU_BASE_SECTION ".data"

#ifndef PER_CPU_DEF_ATTRIBUTES
#define PER_CPU_DEF_ATTRIBUTES
#endif

#ifndef PER_CPU_ATTRIBUTES
#define PER_CPU_ATTRIBUTES
#endif


#define __PCPU_ATTRS(sec)    \
	__attribute__((section(PER_CPU_BASE_SECTION sec)))   \
    PER_CPU_ATTRIBUTES

#define DECLARE_PER_CPU_SECTION(type, name, sec)    \
	extern __PCPU_ATTRS(sec) __typeof__(type) name

#define DEFINE_PER_CPU_SECTION(type, name, sec)    \
	__PCPU_ATTRS(sec) PER_CPU_DEF_ATTRIBUTES    \
    __typeof__(type) name

#define DECLARE_PER_CPU(type, name)    \
	DECLARE_PER_CPU_SECTION(type, name, "")
/* __attribute__((section(".data" ""))) __typeof__(type) name */
#define DEFINE_PER_CPU(type, name)    \
	DEFINE_PER_CPU_SECTION(type, name, "")

/* Don't support SMP */
#define __verify_pcpu_ptr(ptr) do {     \
	const void *__vpp_verify = (typeof(ptr))NULL;   \
	(void) __vpp_verify; } while (0)

#define VERIFY_PERCPU_PTR(__p) ({    \
		__verify_pcpu_ptr((__p));     \
		(typeof(*(__p)) *)(__p); })

#define per_cpu_ptr(ptr, cpu) ({ (void)(cpu); VERIFY_PERCPU_PTR((ptr)); })
/* 
 * #define per_cpu_ptr(ptr, cpu)
 * ({
 *   (void)(cpu);
 *   ({
 *     do {
 *         const void *__vpp_verify = (typeof(ptr))NULL;
 *         (void) __vpp_verify;
 *     } while (0);
 *     (typeof(*(ptr)) *) (ptr);
 *   });
 *  })
 *
 */

/* asm-generic/percpu.h */

#define this_cpu_ptr(ptr)    per_cpu_ptr(ptr, 0)
#define __this_cpu_ptr(ptr)  this_cpu_ptr(ptr)
/* end -- asm-generic/percpu.h */


#define __this_cpu_generic_to_op(pcp, val, op)    \
	do {           \
		*__this_cpu_ptr(&(pcp)) op val;           \
	} while (0)

#define _this_cpu_generic_to_op(pcp, val, op)    \
	do {                                         \
		preempt_disable();                       \
		*__this_cpu_ptr(&(pcp)) op val;          \
		preempt_enable();                        \
	} while (0)

extern void __bad_size_call_parameter(void);

	     /*
	     BUILD_BUG_ON(sizeof(pcp1) != sizeof(pcp2));             \
	     VM_BUG_ON((unsigned long)(&pcp1) % (2 * sizeof(pcp1))); \
	     VM_BUG_ON((unsigned long)(&pcp2) !=                     \
			       (unsigned long)(&pcp1) + sizeof(pcp1));       \
	     */
#define __pcpu_double_call_return_bool(stem, pcp1, pcp2, ...)    \
	({                                                           \
	     bool pdcrb_ret__;                                       \
	     __verify_pcpu_ptr(&pcp1);                               \
	     /*                                                      \
	     BUILD_BUG_ON(sizeof(pcp1) != sizeof(pcp2));             \
	     VM_BUG_ON((unsigned long)(&pcp1) % (2 * sizeof(pcp1))); \
	     VM_BUG_ON((unsigned long)(&pcp2) !=                     \
			       (unsigned long)(&pcp1) + sizeof(pcp1));       \
	     */                                                      \
	     switch(sizeof(pcp1)) {                                  \
	     case 1: pdcrb_ret__ = stem##1(pcp1, pcp2, __VA_ARGS__); break;    \
	     case 2: pdcrb_ret__ = stem##2(pcp1, pcp2, __VA_ARGS__); break;    \
	     case 4: pdcrb_ret__ = stem##4(pcp1, pcp2, __VA_ARGS__); break;    \
	     case 8: pdcrb_ret__ = stem##8(pcp1, pcp2, __VA_ARGS__); break;    \
	     default:                                                \
	         __bad_size_call_parameter(); break;                 \
	     }                                                       \
	     pdcrb_ret__;                                                \
	 })


#define __pcpu_size_call(stem, variable, ...)    \
	do {                                         \
        __verify_pcpu_ptr(&(variable));          \
		switch(sizeof(variable)) {               \
			case 1: stem##1(variable, __VA_ARGS__); break;    \
			case 2: stem##2(variable, __VA_ARGS__); break;    \
			case 4: stem##4(variable, __VA_ARGS__); break;    \
			case 8: stem##8(variable, __VA_ARGS__); break;    \
			default:                                          \
				__bad_size_call_parameter(); break;           \
		}                                         \
	} while (0)

#define __pcpu_size_call_return(stem, variable)    \
	({                  \
	     typeof(variable) pscr_ret__;              \
	     __verify_pcpu_ptr(&(variable));           \
	     switch(sizeof(variable)) {              \
		 case 1: pscr_ret__ = stem##1(variable); break;    \
		 case 2: pscr_ret__ = stem##2(variable); break;    \
		 case 4: pscr_ret__ = stem##4(variable); break;    \
		 case 8: pscr_ret__ = stem##8(variable); break;    \
		 default:          \
			 __bad_size_call_parameter(); break; }         \
		 pscr_ret__; })

#define this_cpu_write_1(pcp, val) _this_cpu_generic_to_op((pcp), (val), =)
#define this_cpu_write_2(pcp, val) _this_cpu_generic_to_op((pcp), (val), =)
#define this_cpu_write_4(pcp, val) _this_cpu_generic_to_op((pcp), (val), =)
#define this_cpu_write_8(pcp, val) _this_cpu_generic_to_op((pcp), (val), =)

#define this_cpu_write(pcp, val) __pcpu_size_call(this_cpu_write_, (pcp), (val))


#define __this_cpu_write_1(pcp, val) __this_cpu_generic_to_op((pcp), (val), =)
#define __this_cpu_write_2(pcp, val) __this_cpu_generic_to_op((pcp), (val), =)
#define __this_cpu_write_4(pcp, val) __this_cpu_generic_to_op((pcp), (val), =)
#define __this_cpu_write_8(pcp, val) __this_cpu_generic_to_op((pcp), (val), =)

#define __this_cpu_write(pcp, val)  __pcpu_size_call(__this_cpu_write_, (pcp), (val))

#define __this_cpu_read_1(pcp) (*__this_cpu_ptr(&(pcp)))
#define __this_cpu_read_2(pcp) (*__this_cpu_ptr(&(pcp)))
#define __this_cpu_read_4(pcp) (*__this_cpu_ptr(&(pcp)))
#define __this_cpu_read_8(pcp) (*__this_cpu_ptr(&(pcp)))

#define __this_cpu_read(pcp)  __pcpu_size_call_return(__this_cpu_read_, (pcp))
/*
#define _this_cpu_generic_cmpxchg_double(pcp1, pcp2, oval1, oval2, nval1, nval2)   \
	({                                                                             \
	     int ret__;             \
	     preempt_disable();     \
	     ret__ = __this_cpu_generic_cmpxchg_double(pcp1, pcp2, oval1, oval2, nval1, nval2);   \
	     preempt_enable();       \
	     ret__; })
*/
#define __this_cpu_generic_cmpxchg_double(pcp1, pcp2, oval1, oval2, nval1, nval2)    \
	({                           \
	     int __ret = 0;          \
	     if (__this_cpu_read(pcp1) == (oval1) && __this_cpu_read(pcp2) == (oval2)) {   \
	         __this_cpu_write(pcp1, (nval1));    \
	         __this_cpu_write(pcp2, (nval2));    \
	         __ret = 1;                          \
	     }    \
	     (__ret); })

#define irqsafe_generic_cpu_cmpxchg_double(pcp1, pcp2, oval1, oval2, nval1, nval2)    \
	({                     \
	     int ret__;        \
	     unsigned long flags;     \
	     local_irq_save(flags);   \
	     ret__ = __this_cpu_generic_cmpxchg_double(pcp1, pcp2, oval1, oval2, nval1, nval2);   \
	     local_irq_restore(flags);     \
	     ret__; })

#define irqsafe_cpu_cmpxchg_double_1(pcp1, pcp2, oval1, oval2, nval1, nval2)    \
	irqsafe_generic_cpu_cmpxchg_double(pcp1, pcp2, oval1, oval2, nval1, nval2)

#define irqsafe_cpu_cmpxchg_double_2(pcp1, pcp2, oval1, oval2, nval1, nval2)    \
	irqsafe_generic_cpu_cmpxchg_double(pcp1, pcp2, oval1, oval2, nval1, nval2)

#define irqsafe_cpu_cmpxchg_double_4(pcp1, pcp2, oval1, oval2, nval1, nval2)    \
	irqsafe_generic_cpu_cmpxchg_double(pcp1, pcp2, oval1, oval2, nval1, nval2)

#define irqsafe_cpu_cmpxchg_double_8(pcp1, pcp2, oval1, oval2, nval1, nval2)    \
	irqsafe_generic_cpu_cmpxchg_double(pcp1, pcp2, oval1, oval2, nval1, nval2)

#define irqsafe_cpu_cmpxchg_double(pcp1, pcp2, oval1, oval2, nval1, nval2)    \
	__pcpu_double_call_return_bool(irqsafe_cpu_cmpxchg_double_, (pcp1), (pcp2), (oval1), (oval2),    \
			(nval1), (nval2))
#endif
