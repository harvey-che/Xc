#include <Xc/sched.h>

#ifdef CONFIG_FAIR_GROUP_SCHED

#else

static inline struct rq *task_of(struct sched_entity *se)
{
    return container_of(se, struct task_struct, se);
}

static inline struct rq *rq_of(struct cfs_rq *cfs_rq)
{
    return container_of(cfs_rq, struct rq, cfs);
}

#define entity_is_task(se) 1

#define for_each_sched_entity(se)    \
	for (; se; se = NULL)

#endif /* CONFIG_FAIR_GROUP_SCHED */

static int wakeup_preempt_entity(struct sched_entity *curr, struct sched_entity *se)
{
    s64 gran, vdiff = curr->vruntime - se->vruntime;
	if (vdiff <=0)
		return -1;

	gran = wakeup_gran(curr, se);
	if (vdiff > gran)
		return 1;

	return 0;
}

static void set_last_buddy(struct sched_entity *se)
{
    if (entity_is_task(se) && task_of(se)->policy == SCHED_IDLE)
		return;

	for_each_sched_entity(se)
		cfs_rq_of(se)->last = se;
}

static void set_next_buddy(struct sched_entity *se)
{
    if (entity_is_task(se) && task_of(se)->policy == SCHED_IDLE)
		return;

	for_each_sched_entity(se)
		cfs_rq_of(se)->next = se;
}

static void __clear_buddies_last(struct sched_entity *se)
{
    for_each_sched_entity(se) {
        struct cfs_rq *cfs_rq = cfs_rq_of(se);

		if (cfs_rq->last == se)
			cfs_rq->last = NULL;
		else
			break;
	}
}

static void __clear_buddies_next(struct sched_entity *se)
{
    for_each_sched_entity(se) {
        struct cfs_rq *cfs_rq = cfs_rq_of(se);

		if (cfs_rq->next == se)
			cfs_rq->next = NULL;
		else
			break;
	}
}

static void __clear_buddies_skip(struct sched_entity *se)
{
    for_each_sched_entity(se) {
        struct cfs_rq *cfs_rq = cfs_rq_of(se);

		if (cfs_rq->skip == se)
			cfs_rq->skip = NULL;
		else
			break;
	}
}

static void clear_buddies(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
    if (cfs_rq->last == se)
		__clear_buddies_last(se);

	if (cfs_rq->next == se)
		__clear_buddies_next(se);

	if (cfs_rq->skip == se)
		__clear_buddies_skip(se);
}

static void account_entity_enqueue(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
    update_load_add(&cfs_rq->load, se->load.weight);

	if (!parent_entity(se))
		inc_cpu_load(rq_of(cfs_rq), se->load.weight);

	if (entity_is_task(se)) {
        /* add_cfs_task_weight(cfs_rq, se->load.weight); */
		list_add(&se->group_node, &cfs_rq->tasks);
	}
	cfs_rq->nr_running++;
}

static void account_entity_dequeue(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
    update_load_sub(&cfs_rq->load, se->load.weight);

	if (!parent_entity(se))
		dec_cpu_load(rq_of(cfs_rq), se->load.weight);

	if (entity_is_task(se)) {
        /* add_cfs_task_weight(cfs_rq, -se->load.weight); */
		list_del_init(&se->group_node);
	}
	cfs_rq->nr_running--;
}

static void update_min_vruntime(struct cfs_rq *cfs_rq)
{
    u64 vruntime = cfs_rq->min_vruntime;

	if (cfs_rq->curr)
		vruntime = cfs_cq->curr->vruntime;

	if (cfs_rq->rb_leftmost) {
        struct sched_entity *se = rb_entry(cfs_rq->rb_leftmost, struct sched_entity, run_node);

		if (!cfs_rq->curr)
			vruntime = se->vruntime;
		else
			vruntime = min_vruntime(vruntime, se->vruntime);
	}
	cfs_rq->min_vruntime = max_vruntime(cfs_rq->min_vruntime, vruntime);
}

static void __enqueue_entity(struct cfs_rq *cfs_rq, struct shed_entity *se)
{
    struct rb_node **link = &cfs_rq->tasks_timeline.rb_node;
	struct rb_node *parent = NULL;
	struct sched_entity *entry;
	int leftmost = 1;

	while (*link) {
        parent = *link;

		entry = rb_entry(parent, struct sched_entity, run_node);

		if (entity_before(se, entry)) {
            link = &parent->rb_left;
		} else {
            link = &parent->rb_right;
			leftmost = 0;
		}
	}

	if (leftmost)
		cfs_rq->rb_leftmost = &se->run_node;

	rb_link_node(&se->run_node, parent, link);
	rb_insert_color(&se->run_node, &cfs_rq->tasks_timeline);
}

static void place_entity(struct cfs_rq *cfs_rq, struct sched_entity *se, int initial)
{
    u64 vruntime = cfs_rq->min_vruntime;

	if (initial)
		vruntime += sched_vslice(cfs_rq, se);

	if (!initial) {
        unsigned long thresh = sysctl_sched_latency;

	    thresh >>= 1;

		vruntime -= thresh;
	}

	vruntime = max_vruntime(se->vruntime, vruntime);

	se->vruntime = vruntime;
}

static void enqueue_entity(struct cfs_rq *cfs_rq, struct sched_entity *se, int flags)
{
    if (!(flags & ENQUEUE_WAKEUP) || (flags & ENQUEUE_WAKING))
		se->vruntime += cfs_rq->min_vruntime;

	update_curr(cfs_rq);
	/* update_cfs_load(cfs_rq, 0); */
	account_entity_enqueue(cfs_rq, se);
	/* update_cfs_shares(cfs_rq); */

	if (flags & ENQUEUE_WAKEUP) {
        place_entity(cfs_rq, se, 0);
		/* enqueue_sleeper(cfs_rq, se); */
	}

	/* update_states_enqueue(cfs_rq, se); */
	/* check_spread(cfs_rq, se); */
	if (se != cfs_rq->curr)
		__enqueue_entity(cfs_rq, se);
	se->on_rq = 1;

	if (cfs_rq->nr_running == 1) {
		/* list_add_leaf_cfs_rq(cfs_rq); */
	}
}

static void dequeue_entity(struct cfs_rq *cfs_rq, struct sched_entity *se, int flags)
{
    update_curr(cfs_rq);

	/* update_stats_dequeue(cfs_rq, se); */
	if (flags & DEQUEUE_SLEEP) {

	}

	clear_buddies(cfs_rq, se);

	if (se != cfs_rq->curr)
		__dequeue_entity(cfs_rq, se);
	se->on_rq = 0;

	/* update_cfs_load(cfs_rq, 0); */
	account_entity_dequeue(cfs_rq, se);

	if (!(flags & DEQUEUE_SLEEP))
		se->vruntime -= cfs_rq->min_vruntime;

	update_min_vruntime(cfs_rq);
	/* update_cfs_shares(cfs_rq); */
}

static inline u64 max_vruntime(u64 min_vruntime, u64 vruntime)
{
    s64 delta = (s64)(vruntime - min_vruntime);
	if (delta > 0)
		min_vruntime = vruntime;
	return min_vruntime;
}

static inline u64 min_vruntime(u64 min_vruntime, u64 vruntime)
{
    s64 delta = (s64)(vruntime - min_vruntime);
	if (delta < 0)
		min_vruntime = vruntime;
	return min_vruntime;
}


static u64 sched_slice(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
    u64 slice = __sched_period(cfs_rq->nr_running + !se->on_rq);

	for_each_sched_entity(se) {
        struct load_weight *load;
		struct load_weight lw;

		cfs_rq = cfs_rq_of(se);
		load = &cfs_rq->load;

		if (!se->on_rq) {
            lw = cfs_rq->load;

			update_load_add(&lw, se->load.weight);
			load = &lw;
		}
		slice = calc_delta_mine(slice, se->load.weight, load);
	}
	return slice;
}

static inline void __update_curr(struct cfs_rq *cfs_rq, struct sched_entity *curr, 
		                         unsigned long delta_exec)
{
    unsigned long delta_exec_weighted;

	curr->sum_exec_runtime += delta_exec;

	delta_exec_weighted = calc_delta_fair(delta_exec, curr);

	curr->vruntime += delta_exec_weighted;

	update_min_vruntime(cfs_rq);
}

static void update_curr(struct cfs_rq *cfs_rq)
{
    struct sched_entity *curr = cfs_rq->curr;
	u64 now = rq_of(cfs_rq)->clock_task;
	unsigned long delta_exec;

    if (!curr)
		return;

	delta_exec = (unsigned long)(now - curr->exec_start);
	if (!delta_exec)
		return;

	__update_curr(cfs_rq, curr, delta_exec);
	curr->exec_start = now;

	if (entity_is_task(curr)) {
        struct task_struct *curtask = task_of(curr);

		/* cpuacct_charge(curtask, delta_exec);  */
		/* account_group_exec_runtime(curtask, delta_exec); */
	}
}

static inline unsigned long calc_delta_fair(unsigned long delta, struct sched_entity *se)
{
    if (se->load.weight != NICE_0_LOAD)
		delta = calc_delta_mine(delta, NICE_0_LOAD, &se->load);

	return delta;
}

static unsigned long wakeup_gran(struct sched_entity *curr, struct sched_entity *se)
{
    unsigned long gran = sysctl_sched_wakeup_granularity;

	return calc_delta_fair(gran, se);
}

static int wakeup_preempt_entity(struct sched_entity *curr, struct sched_entity *se)
{
    s64 gran, vdiff = curr->vruntime - se->vruntime;

	if (vdiff <= 0)
		return -1;

	gran = wakeup_gran(curr, se);
	if (vdiff >  gran)
		return 1;

	return 0;
}

static void check_preempt_wakeup(struct rq *rq, struct task_struct *p, int wake_falgs)
{
    struct task_struct *curr = rq->curr;
	struct sched_entity *se = &curr->se, *pse = &p->se;
	struct cfs_rq *cfs_rq = task_cfs_rq(curr);
	int scale = cfs_rq->nr_running >= sched_nr_latency;
	int next_buddy_marked = 0;

	if (se == pse)
		return;

    if (scale && !(wake_flags & WF_FOR)) {
        set_next_buddy(pse);
		next_buddy_marked = 1;
	}

	if (test_tsk_need_resched(curr))
		return;

	if (curr->policy == SCHED_IDLE && p->policy != SCHED_IDLE)
		goto preempt;

	if (p->policy != SCHED_NORMAL)
		return;

	/* find_matching_se(&se, &pse); */
	update_curr(cfs_rg_of(se));
	/* BUG_ON(!pse); */
	if (wakeup_preempt_entity(se, pse) == 1) {
        if (!next_buddy_marked)
			set_next_buddy(pse);
		goto preempt;
	}

	return;

preempt:
	resched_task(curr);

	if (!se->on_rq || curr == rq->idle)
		return;

	if (scale && entity_is_task(se))
		set_last_buddy(se);
}

static struct sched_entity *__pick_first_entity(struct cfs_rq *cfs_rq)
{
    struct rb_node *left = cfs_rq->rb_leftmost;
	if (!left)
		return NULL;

	return rb_entry(left, struct sched_entity, run_node);
}


static void check_preempt_tick(struct cfs_rq *cfs_rq, struct sched_entity *curr)
{
    unsigned long ideal_runtime, delta_exec;

    ideal_runtime = sched_slice(cfs_rq, curr);
    delta_exec = curr->sum_exec_runtime - curr->prev_sum_exec_runtime;
    if (delta_exec > ideal_runtime) {
       resched_task(rq_of(cfs_rq)->curr);

	   clear_buddies(cfs_rq, curr);
	   return;
    }

	if (delta_exec < sysctl_sched_min_granularity)
		return;

	if (cfs_rq->nr_running > 1) {
        struct sched_entity *se = __pick_first_entity(cfs_rq);
		s64 delta = curr->vruntime - se->vruntime;

		if (delta < 0)
			return;

		if (delta > ideal_runtime)
			resched_task(rq_of(cfs_rq)->curr);
	}
}

static void entity_tick(struct cfs_rq *cfs_rq, struct sched_entity *curr, int queued)
{
    update_curr(cfs_rq);

	/* update_entity_shares_tick(cfs_rq); */

	if (cfs_rq->nr_running > 1)
		check_preempt_tick(cfs_rq, curr);
}

static void task_tick_fair(struct rq *rq, struct task_struct *curr, int queued)
{
    struct cfs_rq *cfs_rq;
	struct sched_entity *se = &curr->se;

	for_each_sched_entity(se) {
        cfs_rq = cfs_rq_of(se);
		entity_tick(cfs_rq, se, queued);
	}
}

static void enqueue_task_fair(struct rq *rq, struct task_struct *p, int flags)
{
    struct cfs_rq *cfs_rq;
	struct sched_entity *se = &p->se;

	for_each_sched_entity(se) {
        if (se->on_rq)
			break;

		cfs_rq = cfs_rq_of(se);
		enqueue_entity(cfs_rq, se, flags);
		flags = ENQUEUE_WAKEUP;
	}

	for_each_sched_entity(se) {
        cfs_rq = cfs_rq_of(se);

		update_cfs_load(cfs_rq, 0);
		update_cfs_shares(cfs_rq);
	}

	/* hrtick_update(rq); */
}

static void dequeue_task_fair(struct rq *rq, struct task_struct *p, int flags)
{
    struct cfs_rq *cfs_rq;
    struct sched_entity *se = &p->se;
	int task_sleep = flags & DEQUEUE_SLEEP;

	for_each_sched_entity(se) {
        cfs_rq = cfs_rq_of(se);
		dequeue_entity(cfs_rq, se, flags);

		if (cfs_rq->load.weight) {
            if (task_sleep && parent_entity(se))
				set_next_buddy(parent_entity(se));

			se = parent_entity(se);
			break;
		}
		flags |= DEQUEUE_SLEEP;
	}

	for_each_sched_entity(se) {
        cfs_rq = cfs_rq_of(se);

		update_cfs_load(cfs_rq, 0);
		update_cfs_shares(cfs_rq);
	}

	/* hrtick_update(rq); */
}
