#ifndef _XC_INIT_H
#define _XC_INIT_H

#define __init
#define __init_memblock
#define __cpuinit
#define __initdata

extern void (* late_time_init)(void);

#endif
