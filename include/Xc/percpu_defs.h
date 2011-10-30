#ifndef _XC_PERCPU_DEFS_H
#define _XC_PERCPU_DEFS_H

/* Don't support SMP */
#define PER_CPU_BASE_SECTION ".data"

#ifndef PER_CPU_DEF_ATTRIBUTES
#define PER_CPU_DEF_ATTRIBUTES
#endif

#ifndef PER_CPU_ATTRIBUTES
#define PER_CPU_ATTRIBUTES
#endif

#define __percpu

#define __PCPU_ATTRS(sec)    \
	__percpu __attribute__((section(PER_CPU_BASE_SECTION sec)))   \
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

#endif
