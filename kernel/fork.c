#include <Xc/init.h>
#include <Xc/sched.h>
#include <Xc/kthread.h>
#include <Xc/slab.h>
#include <asm/processor.h>
#include <Xc/mm.h>
#include <Xc/magic.h>
#include <Xc/errno.h>
#include <asm/pgalloc.h>
#include <asm/ptrace.h>
#include <Xc/pid.h>

int nr_threads;
int max_threads;

static struct kmem_cache *task_struct_cachep;

struct kmem_cache *mm_cachep;

void __init for_init(unsigned long mempages)
{
	/*
    task_struct_cachep = kmem_cache_create("task_struct", sizeof(struct task_struct), ARCH_MIN_TASKALIGN, SLAB_PANIC | SLAB_NOTRACK, NULL);
    */
	/* arch_task_cache_init(); */

	max_threads = mempages / (8 * THREAD_SIZE / PAGE_SIZE);

	if (max_threads < 20)
		max_threads = 20;

}


#define alloc_task_struct_node(node)    \
	kmem_cache_alloc_node(task_struct_cachep, GFP_KERNEL, (node))

#define free_task_struct(tsk)    \
	kmem_cache_free(task_struct_cachep, (tsk))

#define allocate_mm() (kmem_cache_alloc(mm_cachep, GFP_KERNEL))
#define free_mm(mm) (kmem_cache_free(mm_cachep, (mm)))

static struct task_struct *dup_task_struct(struct task_struct *orig)
{
    struct task_struct *tsk;
	struct thread_info *ti;
	unsigned long *stackend;
	int node = tsk_fork_get_node(orig);
	int err;

	prepare_to_copy(orig);
	tsk = alloc_task_struct_node(node);
	if (!tsk)
		return NULL;

	ti = alloc_thread_info_node(tsk, node);
	if (!ti) {
		free_task_struct(tsk);
	    return NULL;
	}

	err = arch_dup_task_struct(tsk, orig);
	if (err)
		goto out;

	tsk->stack = ti;

	setup_thread_stack(tsk, orig);
	
	clear_tsk_need_resched(tsk);
	stackend = end_of_stack(tsk);
	*stackend = STACK_END_MAGIC;

	atomic_set(&tsk->usage, 2);

	tsk->splice_pipe = NULL;

	return tsk;

out:
	free_thread_info(ti);
	free_task_struct(tsk);
	return NULL;
}

static inline int mm_alloc_pgd(struct mm_struct *mm)
{
    mm->pgd = pgd_alloc(mm);
	if (!mm->pgd)
		return -ENOMEM;
	return 0;
}

static inline void mm_free_pgd(struct mm_struct *mm)
{
    pgd_free(mm, mm->pgd);
}

static int dup_mmap(struct mm_struct *mm, struct mm_struct *oldmm)
{
    int retval;

	return retval;
}


static unsigned long default_dump_filter = MMF_DUMP_FILTER_DEFAULT;

static struct mm_struct *mm_init(struct mm_struct *mm, struct task_struct *p)
{
    atomic_set(&mm->mm_users, 1);
	atomic_set(&mm->mm_count, 1);
	//init_rwsem(&mm->mmap_sem);
	INIT_LIST_HEAD(&mm->mmlist);
	mm->flags = (current->mm) ? (current->mm->flags & MMF_INIT_MASK) : default_dump_filter;

	//mm->core_state = NULL;
	mm->nr_ptes = 0;

	spin_lock_init(&mm->page_table_lock);
	mm->free_area_cache = TASK_UNMAPPED_BASE;
	mm->cached_hole_size = ~0UL;

	/* mm_init_owner(mm, p); */
	atomic_set(&mm->oom_disable_count, 0);

	if (!mm_alloc_pgd(mm)) {
        mm->def_flags = 0;
		return mm;
	}

	free_mm(mm);
	return NULL;
}

void mmput(struct mm_struct *mm)
{
    
}

struct mm_struct *dup_mm(struct task_struct *tsk)
{
    struct mm_struct *mm, *oldmm = current->mm;
	int err;

	if (!oldmm)
		return NULL;

	mm = allocate_mm();
	if (!mm)
		goto fail_nomem;

	memcpy(mm, oldmm, sizeof(*mm));
	/* mm_init_cpumask(mm); */

	mm->token_priority = 0;
	mm->last_interval = 0;

	if (!mm_init(mm, tsk))
		goto fail_nomem;

	/*
	if (init_new_context(tsk, mm))
		goto fail_nocontext;
	*/

	/* err = dup_mmap(mm, oldmm);
	if (err)
		goto free_pt;
	*/

	mm->hiwater_vm = mm->total_vm;

	return mm;

free_pt:
	//mm->binfmt = NULL;
	mmput(mm);

fail_nomem:
	return NULL;

fail_nocontext:
	mm_free_pgd(mm);
	free_mm(mm);
	return NULL;
}

static int copy_mm(unsigned long clone_flags, struct task_struct *tsk)
{
    struct mm_struct *mm, *oldmm;
	int retval;

	tsk->min_flt = tsk->maj_flt = 0;
	tsk->nvcsw = tsk->nivcsw = 0;

	tsk->mm = NULL;
	tsk->active_mm = NULL;

	oldmm = current->mm;
	if (!oldmm)
		return 0;

	if (clone_flags & CLONE_VM) {
        atomic_inc(&oldmm->mm_users);
		mm = oldmm;
		goto good_mm;
	}

	retval = -ENOMEM;
	mm = dup_mm(tsk);
	if (!mm)
		goto fail_nomem;

good_mm:
	mm->token_priority = 0;
	mm->last_interval = 0;
	
	tsk->mm = mm;
	tsk->active_mm = mm;
	return 0;

fail_nomem:
	return retval;
}

static void copy_flags(unsigned long clone_flags, struct task_struct *p)
{
    unsigned long new_flags = p->flags;

	new_flags &= ~(PF_SUPERPRIV | PF_WQ_WORKER);
	new_flags |= PF_FORKNOEXEC;
	new_flags |= PF_STARTING;
	p->flags = new_flags;
	/* clear_freeze_flag(p); */
}

static void rt_mutex_init_task(struct task_struct *p)
{
    raw_spin_lock_init(&p->pi_lock);
}

static struct task_struct *copy_process(unsigned long clone_flags, unsigned long stack_start, 
		                                struct pt_regs *regs, unsigned long stack_size,
										int *child_tidptr, struct pid *pid, int trace)
{
    int retval;
	struct task_struct *p;
	int cgroup_callbacks_done = 1;
	
	retval = -ENOMEM;
	p = dup_task_struct(current);
	if (!p)
		goto fork_out;

	rt_mutex_init_task(p);

	retval = -EAGAIN;

	current->flags &= ~PF_NPROC_EXCEEDED;

	retval = -EAGAIN;
	if (nr_threads >= max_threads)
		goto bad_fork_cleanup_count;

	p->did_exec = 0;

	copy_flags(clone_flags, p);
	INIT_LIST_HEAD(&p->children);
	INIT_LIST_HEAD(&p->sibling);
	rcu_copy_process(p);
	//p->vfork_done = NULL;

	spin_lock_init(&p->alloc_lock);

	/*
	p->utime = cputime_zero;
	p->stime = cputime_zero;
	p->gtime = cputime_zero;
	p->utimescaled = cputime_zero;
	p->stimescaled = cputime_zero;

	p->prev_utime = cputime_zero;
	p->prev_stime = cputime_zero;

	p->default_timer_slack_ns = current->timer_slack_ns;
    */
	//do_posix_clock_monotonic_gettime(&p->start_time);
	//p->real_start_time = p->start_time;
	//monotonic_to_bootbased(&p->real_start_time);

	//p->io_context = NULL;

	/*
	if (clone_flags & CLONE_THREAD)
		threadgroup_fork_read_lock(current);
    */

	sched_fork(p);

	retval = copy_mm(clone_flags, p);
	if (retval)
		goto bad_fork_cleanup_mm;

	retval = copy_thread(clone_flags, stack_start, stack_size, p, regs);
	if (retval)
		goto bad_fork_cleanup_io;

	if (pid != &init_struct_pid) {
        retval = -ENOMEM;
		pid = alloc_pid(p->nsproxy->pid_ns);
		if (!pid)
			goto bad_fork_cleanup_io;
	}

	p->pid = pid_nr(pid);
	p->tgid = p->pid;
	if (clone_flags & CLONE_THREAD)
		p->tgid = current->tgid;

    p->exit_signal = (clone_flags & CLONE_THREAD) ? -1 : (clone_flags & CSIGNAL);
	
	p->group_leader = p;
	INIT_LIST_HEAD(&p->thread_group);

	cgroup_callbacks_done = 1;

bad_fork_free_pid:
	if (pid != &init_struct_pid)
		free_pid(pid);
bad_fork_cleanup_io:
	if (p->io_context)
		exit_io_context(p);

bad_fork_cleanup_mm:
	if (p->mm) {
        mmput(p->mm);
	}

bad_fork_free:
	free_task(p);
fork_out:
	return ERR_PTR(retval);
}



long do_fork(unsigned long clone_flags, unsigned long stack_start, struct pt_regs *regs, 
		     unsigned long stack_size, int *parent_tidptr, int *child_tidptr)
{
    struct task_struct *p;
	int trace = 0;
	long nr;

	p = copy_process(clone_flags, stack_start, regs, stack_size, child_tidptr, NULL, trace);

	if (!IS_ERR(p)) {
		nr = task_pid_vnr(p);

        p->flags &= ~PF_STARTING;

		wake_up_new_task(p);
	}
	return nr;
}
