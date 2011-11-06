#ifndef _XC_CPUMASK_H
#define _XC_CPUMASK_H

#include <Xc/threads.h>

#if NR_CPUS == 1

#define nr_cpu_ids 1

#define for_each_cpu(cpu, mask)    \
	for ((cpu) = 0; (cpu) < 1; (cpu)++, (void)mask)

#define for_each_possible_cpu(cpu)    for_each_cpu((cpu), NULL/*cpu_possible_mask*/)

#endif

#endif
