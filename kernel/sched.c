#include <Xc/sched.h>
#include <Xc/preempt.h>


#include <Xc/irqflags.h>
#include <Xc/ktime.h>
#include <Xc/hrtimer.h>
#include <Xc/atomic.h>
#include <Xc/list.h>
#include <Xc/init.h>
#include <Xc/time.h>
#include <Xc/gfp.h>

/* sched_clock.c
 *
 * CONFIG_HAVE_UNSTABLE_SCHED_CLOCK is not defined
 */

#if 0

int sched_clock_running;

unsigned long long __attribute__((weak)) sched_clock(void)
{
    return (unsigned long long)(jiffies - INITIAL_JIFFIES) * (NSEC_PER_SEC / HZ);
}

u64 sched_clock_cpu(int cpu)
{
    if (!sched_clock_running)
		return 0;

	return sched_clock();
}

/* end -- sched_clock.c */

struct rt_prio_array {
    DECLARE_BITMAP(bitmap, MAX_RT_PRIO + 1);
	struct list_head queue[MAX_RT_PRIO];
};

struct rt_bandwidth {
    raw_spinlock_t rt_runtime_lock;
	ktime_t        rt_period;
	u64            rt_runtime;
	struct hrtimer rt_period_timer;
};

static struct rt_bandwidth def_rt_bandwidth;


/* CFS-related fields in a runqueue */
struct cfs_rq {
	struct load_weight load;
	unsigned long nr_running;

	u64 exec_clock;
	u64 min_vruntime;

	struct rb_root tasks_timeline;
	struct rb_node *rb_leftmost;

	struct list_head tasks;
	struct list_head *balance_iterator;

	/*
	 * 'curr' points to currently running entity on this cfs_rq.
	 * It is set to NULL otherwise (i.e when none are currently running).
	 */
	struct sched_entity *curr, *next, *last;

	unsigned int nr_spread_over;

#ifdef CONFIG_FAIR_GROUP_SCHED
	struct rq *rq;	/* cpu runqueue to which this cfs_rq is attached */

	/*
	 * leaf cfs_rqs are those that hold tasks (lowest schedulable entity in
	 * a hierarchy). Non-leaf lrqs hold other higher schedulable entities
	 * (like users, containers etc.)
	 *
	 * leaf_cfs_rq_list ties together list of leaf cfs_rq's in a cpu. This
	 * list is used during load balance.
	 */
	int on_list;
	struct list_head leaf_cfs_rq_list;
	struct task_group *tg;	/* group that "owns" this runqueue */

#ifdef CONFIG_SMP
	/*
	 * the part of load.weight contributed by tasks
	 */
	unsigned long task_weight;

	/*
	 *   h_load = weight * f(tg)
	 *
	 * Where f(tg) is the recursive weight fraction assigned to
	 * this group.
	 */
	unsigned long h_load;

	/*
	 * Maintaining per-cpu shares distribution for group scheduling
	 *
	 * load_stamp is the last time we updated the load average
	 * load_last is the last time we updated the load average and saw load
	 * load_unacc_exec_time is currently unaccounted execution time
	 */
	u64 load_avg;
	u64 load_period;
	u64 load_stamp, load_last, load_unacc_exec_time;

	unsigned long load_contribution;
#endif
#endif
};

/* Real-Time classes' related field in a runqueue: */
struct rt_rq {
	struct rt_prio_array active;
	unsigned long rt_nr_running;
#if defined CONFIG_SMP || defined CONFIG_RT_GROUP_SCHED
	struct {
		int curr; /* highest queued rt task prio */
#ifdef CONFIG_SMP
		int next; /* next highest */
#endif
	} highest_prio;
#endif
#ifdef CONFIG_SMP
	unsigned long rt_nr_migratory;
	unsigned long rt_nr_total;
	int overloaded;
	struct plist_head pushable_tasks;
#endif
	int rt_throttled;
	u64 rt_time;
	u64 rt_runtime;
	/* Nests inside the rq lock: */
	raw_spinlock_t rt_runtime_lock;

#ifdef CONFIG_RT_GROUP_SCHED
	unsigned long rt_nr_boosted;

	struct rq *rq;
	struct list_head leaf_rt_rq_list;
	struct task_group *tg;
#endif
};

struct rq {
	/* runqueue lock: */
	raw_spinlock_t lock;

	/*
	 * nr_running and cpu_load should be in the same cacheline because
	 * remote CPUs use both these fields when doing load calculation.
	 */
	unsigned long nr_running;
	#define CPU_LOAD_IDX_MAX 5
	unsigned long cpu_load[CPU_LOAD_IDX_MAX];
	unsigned long last_load_update_tick;
#ifdef CONFIG_NO_HZ
	u64 nohz_stamp;
	unsigned char nohz_balance_kick;
#endif
	unsigned int skip_clock_update;

	/* capture load from *all* tasks on this cpu: */
	struct load_weight load;
	unsigned long nr_load_updates;
	u64 nr_switches;

	struct cfs_rq cfs;
	struct rt_rq rt;

#ifdef CONFIG_FAIR_GROUP_SCHED
	/* list of leaf cfs_rq on this cpu: */
	struct list_head leaf_cfs_rq_list;
#endif
#ifdef CONFIG_RT_GROUP_SCHED
	struct list_head leaf_rt_rq_list;
#endif

	/*
	 * This is part of a global counter where only the total sum
	 * over all CPUs matters. A task can increase this counter on
	 * one CPU and if it got migrated afterwards it may decrease
	 * it on another CPU. Always updated under the runqueue lock:
	 */
	unsigned long nr_uninterruptible;

	struct task_struct *curr, *idle, *stop;
	unsigned long next_balance;
	struct mm_struct *prev_mm;

	u64 clock;
	u64 clock_task;

	atomic_t nr_iowait;

#ifdef CONFIG_SMP
	struct root_domain *rd;
	struct sched_domain *sd;

	unsigned long cpu_power;

	unsigned char idle_at_tick;
	/* For active balancing */
	int post_schedule;
	int active_balance;
	int push_cpu;
	struct cpu_stop_work active_balance_work;
	/* cpu of this runqueue: */
	int cpu;
	int online;

	unsigned long avg_load_per_task;

	u64 rt_avg;
	u64 age_stamp;
	u64 idle_stamp;
	u64 avg_idle;
#endif

#ifdef CONFIG_IRQ_TIME_ACCOUNTING
	u64 prev_irq_time;
#endif

	/* calc_load related fields */
	unsigned long calc_load_update;
	long calc_load_active;

#ifdef CONFIG_SCHED_HRTICK
#ifdef CONFIG_SMP
	int hrtick_csd_pending;
	struct call_single_data hrtick_csd;
#endif
	struct hrtimer hrtick_timer;
#endif

#ifdef CONFIG_SCHEDSTATS
	/* latency stats */
	struct sched_info rq_sched_info;
	unsigned long long rq_cpu_time;
	/* could above be rq->cfs_rq.exec_clock + rq->rt_rq.rt_runtime ? */

	/* sys_sched_yield() stats */
	unsigned int yld_count;

	/* schedule() stats */
	unsigned int sched_switch;
	unsigned int sched_count;
	unsigned int sched_goidle;

	/* try_to_wake_up() stats */
	unsigned int ttwu_count;
	unsigned int ttwu_local;
#endif
};


static DEFINE_PER_CPU_SHARED_ALIGNED(struct rq, runqueues);

#define cpu_rq(cpu)      (&per_cpu(runqueues, (cpu)))
#define cpu_curr(cpu)    (cpu_rq(cpu)->curr)

int idle_cpu(int cpu)
{
    return cpu_curr(cpu) == cpu_rq(cpu)->idle;
}

static inline void update_load_add(struct load_weight *lw, unsigned long inc)
{
	lw->weight += inc;
	lw->inv_weight = 0;
}

static inline void update_load_sub(struct load_weight *lw, unsigned long dec)
{
	lw->weight -= dec;
	lw->inv_weight = 0;
}

static inline void update_load_set(struct load_weight *lw, unsigned long w)
{
	lw->weight = w;
	lw->inv_weight = 0;
}

#define WEIGHT_IDLEPRIO    3
#define WMULT_IDLEPRIO     1431655765

static const int prio_to_weight[40] = {
 /* -20 */     88761,     71755,     56483,     46273,     36291,
 /* -15 */     29154,     23254,     18705,     14949,     11916,
 /* -10 */      9548,      7620,      6100,      4904,      3906,
 /*  -5 */      3121,      2501,      1991,      1586,      1277,
 /*   0 */      1024,       820,       655,       526,       423,
 /*   5 */       335,       272,       215,       172,       137,
 /*  10 */       110,        87,        70,        56,        45,
 /*  15 */        36,        29,        23,        18,        15,
};

static const u32 prio_to_wmult[40] = {
 /* -20 */     48388,     59856,     76040,     92818,    118348,
 /* -15 */    147320,    184698,    229616,    287308,    360437,
 /* -10 */    449829,    563644,    704093,    875809,   1099582,
 /*  -5 */   1376151,   1717300,   2157191,   2708050,   3363326,
 /*   0 */   4194304,   5237765,   6557202,   8165337,  10153587,
 /*   5 */  12820798,  15790321,  19976592,  24970740,  31350126,
 /*  10 */  39045157,  49367440,  61356676,  76695844,  95443717,
 /*  15 */ 119304647, 148102320, 186737708, 238609294, 286331153,
};

static inline void inc_cpu_load(struct rq *rq, unsigned long load)
{
	update_load_add(&rq->load, load);
}

static inline void dec_cpu_load(struct rq *rq, unsigned long load)
{
    update_load_sub(&rq->load, load);
}

static unsigned long calc_delta_mine(unsigned long delta_exec, unsigned long weight, struct load_weight *lw)
{
    u64 tmp;

	if (weight > (1UL << SCHED_LOAD_RESOLUTION))
		tmp = (u64)delta_exec * scale_load_down(weight);
	else
		tmp = (u64)delta_exec;

	if (!lw->inv_weight) {
        unsigned long w = scale_load_down(lw->weight);

		if (BITS_PER_LONG > 32 && w >= WMULT_CONST)
			lw->inv_weight = 1;
		else if (!w)
			lw->inv_weight = WMULT_CONST;
        else
			lw->inv_weight = WMULT_CONST / w;
	}

	if (tmp > WMULT_CONST)
		tmp = SRR(SRR(tmp, WMULT_SHIFT / 2) * lw->inv_weight, WMULT_SHIFT / 2);
	else
		tmp = SRR(tmp * lw->inv_weight, WMULT_SHIFT);

	return (unsigned long)min(tmp, (u64)(unsigned long)LONG_MAX);
}

static inline int cpu_of(struct rq *rq)
{
    return 0;
}

static void update_rq_clock_task(struct rq *rq, s64 delta)
{
    rq->clock_task += delta;
}

static void update_rq_clock(struct rq *rq)
{
    s64 delta;
	if (rq->skip_clock_update > 0)
		return;

	delta = sched_clock_cpu(cpu_of(rq)) - rq->clock;
	rq->clock += delta;
	update_rq_clock_task(rq, delta);
}

static inline struct rq *__task_rq_lock(struct task_struct *p)
{
    struct rq *rq;

	for (;;) {
        rq = task_rq(p);
		raw_spin_lock(&rq->lock);
		if (rq == task_rq(p))
			return rq;
		raw_spin_unlock(&rq->lock);
	}
}

static struct rq *task_rq_lock(struct task_struct *p, unsigned long *flags)
{
    struct rq *rq;

	for (;;) {
        raw_spin_lock_irqsave(&p->pi_lock, *flags);
		rq = task_rq(p);
		raw_spin_lock(&rq->lock);
		if (rq == task_rq(p))
			return rq;
		raw_spin_lock(&rq->lock);
		raw_spin_unlock_irqrestore(&p->pi_lock, *flags);
	}
}

static void __task_rq_unlock(struct rq *rq)
{
    raw_spin_unlock(&rq->lock);
}

static inline void task_rq_unlock(struct rq *rq, struct task_struct *p, unsigned long *flags)
{
    raw_spin_unlock(&rq->lock);
	raw_spin_unlock_irqsave(&p->pi_lock, *flags);
}

static void inc_nr_running(struct rq *rq)
{
    rq->nr_running++;
}

static void dec_nr_running(struct rq *rq)
{
    rq->nr_running--;
}

static void enqueue_task(struct rq *rq, struct task_struct *p, int flags)
{
    update_rq_clock(rq);
	// sched_info_queued(p); 
	p->sched_class->enqueue_task(rq, p, flags);
}

static void dequeue_task(struct rq *rq, struct task_struct *p, int flags)
{
    update_rq_clock(rq);
	// sched_info_queue(p); 
	p->sched_class->dequeue_task(rq, p, flags);
}


static void activate_task(struct rq *rq, struct task_struct *p, int flags)
{
    if (task_contributes_to_load(p))
		rq->nr_uninterruptible--;

	enqueue_task(rq, p, flags);
	inc_nr_running(rq);
}

static void deactivate_task(struct rq *rq, struct task_struct *p, int flags)
{
    if (task_contributes_load(p))
		rq->nr_uninterruptible++;
	dequeue_task(rq, p, flags);
	dec_nr_running(rq);
}

void wake_up_new_task(struct task_struct *p)
{
    unsigned long flags;
	struct rq *rq;

	raw_spin_lock_irqsave(&p->pi_lock, flags);

	rq = __task_rq_lock(p);
	activate_task(rq, p, 0);
	p->on_rq = 1;
	
	check_preempt_curr(rq, p, WF_FORK);

	task_rq_unlock(rq, p, &flags);
}

#endif

static void __schedule(void)
{
	/*
    struct task_struct *prev, *next;
	unsigned long *switch_count;
    struct rq *rq;
	int cpu;

need_resched:
	preempt_disable();
	cpu = smp_processor_id();

	prev = rq->curr;

	raw_spin_lock_irq(&rq->lock);

	switch_count = &prev->nivcsw;
	if (prev->state && !(preempt_count() & PREEMPT_ACTIVE)) {
        if (signal_pending_state(prev->state, prev)) {
            prev->state = TASK_RUNNING;
		} else {
            deactivate_task(rq, prev, DEQUEUE_SLEEP);
			prev->on_rq = 0;

			if (prev->flags & PF_WQ_WORKER) {
                struct task_struct *to_wakeup;

				to_wakeup = wq_worker_sleeping(prev, cpu);
				if (to_wakeup)
					try_to_wake_up_local(to_wakeup);
			}
		}
		switch_count = &prev->nvcsw;
	}

	//pre_schedule(rq, prev);
	
	if (!rq->nr_running)
		idle_balance(cpu, rq);

	put_prev_task(rq, prev);
	next = pick_next_task(rq);
	clear_tsk_need_resched(prev);
	rq->skip_clock_update = 0;

	if (prev != next) {
        rq->nr_switches++;
		rq->curr = next;
		++*switch_count;

		context_switch(rq, prev, next);

		cpu = smp_processor_id();
		rq = cpu_rq(cpu);
	} else
		raw_spin_unlock_irq(&rq->lock);

	//post_schedule(rq);

	preempt_enable_no_resched();
	if (need_resched())
		goto need_resched;
	*/
}

static inline void sched_submit_work(struct task_struct *tsk)
{
	if (!tsk->state)
		return;

	/*
	if (blk_needs_flush_plug(tsk))
		blk_schedule_flush_plug(tsk);
	*/
}

void schedule(void)
{
    struct task_struct *tsk = current;

	sched_submit_work(tsk);
	__schedule();
}

static void __cond_resched(void)
{
    add_preempt_count(PREEMPT_ACTIVE);
	__schedule();
	sub_preempt_count(PREEMPT_ACTIVE);
}

static inline int should_resched(void)
{
    return need_resched() && !(preempt_count() & PREEMPT_ACTIVE);
}

int _cond_resched(void)
{
    if (should_resched()) {
        __cond_resched();
		return 1;
	}
	return 0;
}

void preempt_schedule(void)
{
    struct thread_info *ti = current_thread_info();

	if (ti->preempt_count || irqs_disabled())
		return;

	do {
		add_preempt_count_notrace(PREEMPT_ACTIVE);
		__schedule();
		sub_preempt_count_notrace(PREEMPT_ACTIVE);
		
		barrier();
	} while (need_resched());
}

void preempt_schedule_irq(void)
{
    struct thread_info *ti = current_thread_info();

	/* BUG_ON(ti->preempt_count || !irqs_disabled()); */
	do {
		add_preempt_count(PREEMPT_ACTIVE);
		local_irq_enable();
		__schedule();
		local_irq_disable();
		sub_preempt_count(PREEMPT_ACTIVE);

		/* What is this for? */
		barrier();
	} while (need_resched());
}

/*
static void init_cfs_rq(struct cfs_rq *cfs_rq)
{
    cfs_rq->tasks_timeline = RB_ROOT;
	INIT_LIST_HEAD(&cfs_rq->tasks);
	cfs_rq->min_vruntime = (u64)(-(1LL << 20));
}

static void init_rt_rq(struct rt_rq *rt_rq, struct rq *rq)
{
     struct rt_prio_array *array;
	 int i;

	 array = &rt_rq->active;
	 for (i = 0; i < MAX_RT_PRIO; i++) {
        INIT_LIST_HEAD(array->queue + i);
		__clear_bit(i, array->bitmap);
	 }

	 rt_rq->rt_time = 0;
	 rt_rq->rt_throttled = 0;
	 rt_rq->rt_runtime = 0;
	 raw_spin_lock_init(&rt_rq->rt_runtime_lock);
}

// CONFIG_SCHED_HRTICK is not defined
static inline void init_rq_hrtick(struct rq *rq)
{
}

// CONFIG_CGROUP_SCHED is not defined
static inline void set_task_rq(struct task_struct *p, unsigned int cpu) {}

static inline void __set_task_cpu(struct task_struct *p, unsigned int cpu)
{
    set_task_rq(p, cpu);
}

void __cpuinit init_idle(struct task_struct *idle, int cpu)
{
    struct rq *rq = cpu_rq(cpu);
	unsigned long flags;

	raw_spin_lock_irqsave(&rq->lock, flags);

	__sched_fork(idle);
	idle->state = TASK_RUNNING;
	idle->se.exec_start = sched_clock();

	// CONFIG_SMP is not defined
	// do_set_cpus_allowed(idle, cpumask_of(cpu));

	rcu_read_lock();
	__set_task_cpu(idle, cpu);
	rcu_read_unlock();

	rq->curr = rq->idle = idle;

	raw_spin_unlock_irqrestore(&rq->lock, flags);

	task_thread_info(idle)->preempt_count = 0;
	idle->sched_class = &idle_sched_class;
	// ftrace_graph_init_idle_task(idle, cpu);
}


static void init_rt_bandwidth(struct rt_bandwidth *rt_b, u64 period, u64 runtime)
{
    rt_b->rt_period = ns_to_ktime(period);
	rt_b->rt_runtime = runtime;

	raw_spin_lock_init(&rt_b->rt_runtime_lock);

	hrtimer_init(&rt_b->rt_period_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	rt_b->rt_period_timer.function = shed_rt_period_timer;
}

static void set_load_weight(struct task_struct *p)
{
    int prio = p->static_prio - MAX_RT_PRIO;
	struct load_weight *load = &p->se.load;

	if (p->policy == SCHED_IDLE) {
        load->weight = scale_load(WEIGHT_IDLEPRIO);
		load->inv_weight = WMULT_IDLEPRIO;
		return;
	}
	load->weight = scale_load(prio_to_weight[prio]);
	load->inv_weight = prio_to_wmult[prio];
}

void __init sched_init(void)
{
    int i, j;
	unsigned long alloc_size = 0, ptr;

    init_rt_bandwidth(&def_rt_bandwidth, global_rt_period(), global_rt_runtime());

	for_each_possible_cpu(i) {
        struct rq *rq;

		rq = cpu_rq(i);
		raw_spin_lock_init(&rq->lock);
		rq->nr_running = 0;
		rq->calc_load_active = 0;
		rq->calc_load_update = jiffies + LOAD_FREQ;
		init_cfs_rq(&rq->cfs);
		init_rt_rq(&rq->rt, rq);

		rq->rt.rt_runtime = def_rt_bandwidth.rt_runtime;

		for (j = 0; j < CPU_LOAD_IDX_MAX; j++)
			rq->cpu_load[j] = 0;
		
		rq->last_load_update_tick = jiffies;

		init_rq_hrtick(rq);
		atomic_set(&rq->nr_iowait, 0);
	}

	set_load_weight(&init_task);

	atomic_inc(&init_mm.mm_count);
	// CONFIG_SMP is not defined
	// enter_lazy_tlb(&init_mm, current);

	init_idle(current, smp_processor_id());

	calc_load_update = jiffies + LOAD_FREQ;

	current->sched_class = &fair_sched_class;

	zalloc_cpumask_var(&nohz_cpu_mask, GFP_NOWAIT);
	
	scheduler_running = 1;
}
*/

int idle_cpu(int cpu)
{
    return 1;
}
