#ifndef _XC_CPUMASK_H
#define _XC_CPUMASK_H

#include <Xc/threads.h>
#include <Xc/bitmap.h>

typedef struct cpumask { DECLARE_BITMAP(bits, NR_CPUS); } cpumask_t;

#define cpumask_bits(maskp) ((maskp)->bits)

#if NR_CPUS == 1

#define nr_cpu_ids 1

#define for_each_cpu(cpu, mask)    \
	for ((cpu) = 0; (cpu) < 1; (cpu)++, (void)mask)

#define for_each_possible_cpu(cpu)    for_each_cpu((cpu), NULL/*cpu_possible_mask*/)

#endif

/* */
#ifdef CONFIG_CPUMASK_OFFSTACK

#else

#define nr_cpumask_bits NR_CPUS

typedef struct cpumask cpumask_var_t[1];

static inline void cpumask_clear(struct cpumask *dstp)
{
    bitmap_zero(cpumask_bits(dstp), nr_cpumask_bits);
}

static inline bool zalloc_cpumask_var(cpumask_var_t *mask, gfp_t flags)
{
    cpumask_clear(*mask);
	return true;
}

static inline bool alloc_cpumask_var(cpumask_var_t *mask, gfp_t flags)
{
    return true;
}

static inline void free_cpumask_var(cpumask_var_t mask)
{
}

#endif

#endif
