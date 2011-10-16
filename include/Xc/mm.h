#ifndef _XC_MM_H
#define _XC_MM_H

#include <types.h>

/*
 * 1st total_physical_memsize;
 *
 */
extern u32 boot_params[128];

void early_mem_init();
void setup_paging(void);

#endif
