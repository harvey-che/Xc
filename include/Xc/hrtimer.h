#ifndef _XC_HRTIMER_H
#define _XC_HRTIMER_H

#include <Xc/ktime.h>
#include <Xc/timerqueue.h>
#include <Xc/spinlock.h>

enum hrtimer_mode {
    HRTIMER_MODE_ABS = 0x0,
	HRTIMER_MODE_REL = 0x1,
	HRTIMER_MODE_PINNED = 0x2,
	HRTIMER_MODE_ABS_PINNED = 0x2,
	HRTIMER_MODE_REL_PINNED = 0x3,
};

enum hrtimer_restart {
    HRTIMER_NORESTART,
	HRTIMER_RESTART,
};


struct hrtimer_clock_base {
	struct hrtimer_cpu_base	*cpu_base;
	clockid_t		index;
	struct timerqueue_head	active;
	ktime_t			resolution;
	ktime_t			(*get_time)(void);
	ktime_t			softirq_time;
#ifdef CONFIG_HIGH_RES_TIMERS
	ktime_t			offset;
#endif
};

enum hrtimer_base_type {
    HRTIMER_BASE_MONOTONIC,
	HRTIMER_BASE_REALTIME,
	HRTIMER_BASE_BOOTTIME,
	HRTIMER_MAX_CLOCK_BASES,
};


struct hrtimer_cpu_base {
	raw_spinlock_t			lock;
	struct hrtimer_clock_base	clock_base[HRTIMER_MAX_CLOCK_BASES];
#ifdef CONFIG_HIGH_RES_TIMERS
	ktime_t				expires_next;
	int				hres_active;
	int				hang_detected;
	unsigned long			nr_events;
	unsigned long			nr_retries;
	unsigned long			nr_hangs;
	ktime_t				max_hang_time;
#endif
};


struct hrtimer {
	struct timerqueue_node		node;
	ktime_t				_softexpires;
	enum hrtimer_restart		(*function)(struct hrtimer *);
	struct hrtimer_clock_base	*base;
	unsigned long			state;
#ifdef CONFIG_TIMER_STATS
	int				start_pid;
	void				*start_site;
	char				start_comm[16];
#endif
};

#endif
