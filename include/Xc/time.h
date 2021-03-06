#ifndef _XC_TIME_H
#define _XC_TIME_H

#define CLOCK_REALTIME    0
#define CLOCK_MONOTONIC   1
#define CLOCK_PROCESS_CPUTIME_ID  2
#define CLOCK_THREAD_CPUTIME_ID   3
#define CLOCK_MONOTONIC_RAW       4
#define CLOCK_REALTIME_COARSE     5
#define CLOCK_MONOTONIC_COARSE    6
#define CLOCK_BOOTTIME            7
#define CLOCK_REALTIME_ALARM      8
#define CLOCK_BOOTTIME_ALARM      9

/* asm/time.h */
extern void hpet_time_init(void);
/* end -- asm/time.h */

#endif
