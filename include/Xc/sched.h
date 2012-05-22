#ifndef _XC_SCHED_H
#define _XC_SCHED_H

#include <asm/pgtable_types.h>
#include <Xc/thread_info.h>
#include <Xc/current.h>
#include <Xc/spinlock.h>
#include <Xc/cpumask.h>
#include <Xc/rbtree.h>

#define CSIGNAL     0x000000ff
#define CLONE_VM    0x00000100
#define CLONE_FS    0x00000200
#define CLONE_FILES 0x00000400
#define CLONE_SIGHAND 0x00000800
#define CLONE_PTRACE  0x00002000
#define CLONE_VFOR    0x00004000
#define CLONE_PARENT  0x00008000
#define CLONE_THREAD  0x00010000
#define CLONE_NEWNS   0x00020000
#define CLONE_SYSVSEM 0x00040000
#define CLONE_SETTLS  0x00080000
#define CLONE_PARENT_SETTID  0x00100000
#define CLONE_CHILD_CLEARTID 0x00200000
#define CLONE_DETACHED       0x00400000
#define CLONE_UNTRACED       0x00800000
#define CLONE_CHILD_SETTID   0x01000000

#define CLONE_NEWUTS         0x04000000
#define CLONE_NEWIPC         0x08000000
#define CLONE_NEWUSER        0x10000000
#define CLONE_NEWPID         0x20000000
#define CLONE_NEWNET         0x40000000
#define CLONE_IO             0x80000000

union thread_union {
    struct thread_info thread_info;
	unsigned long stack[THREAD_SIZE/sizeof(long)];
};

extern union thread_union init_thread_union;
extern struct mm_struct init_mm;

#define TASK_RUNNING          0
#define TASK_INTERRUPTIBLE    1
#define TASK_UNINTERRUPTIBLE  2

#define TASK_WAKEKILL         128

#define MMF_DUMPABLE 0
#define MMF_DUMP_SECURELY 1

#define MMF_DUMPABLE_BITS 2
#define MMF_DUMPABLE_MASK ((1 << MMF_DUMPABLE_BITS) - 1)

#define MMF_DUMP_ANON_PRIVATE 2
#define MMF_DUMP_ANON_SHARED  3
#define MMF_DUMP_MAPPED_PRIVATE 4
#define MMF_DUMP_MAPPED_SHARED  5
#define MMF_DUMP_ELF_HEADERS    6
#define MMF_DUMP_HUGETLB_PRIVATE 7
#define MMF_DUMP_HUGETLB_SHARED  8

#define MMF_DUMP_FILTER_SHIFT    MMF_DUMPABLE_BITS
#define MMF_DUMP_FILTER_BITS 7
#define MMF_DUMP_FILTER_MASK \
	(((1 << MMF_DUMP_FILTER_BITS) - 1) << MMF_DUMP_FILTER_SHIFT)
#define MMF_DUMP_FILTER_DEFAULT \
	((1 << MMF_DUMP_ANON_PRIVATE) | (1 << MMF_DUMP_ANON_SHARED) | \
	 (1 << MMF_DUMP_HUGETLB_PRIVATE) | MMF_DUMP_MASK_DEFAULT_ELF)

#define MMF_DUMP_MASK_DEFAULT_ELF 0

#define MMF_INIT_MASK (MMF_DUMPABLE_MASK | MMF_DUMP_FILTER_MASK)

#define PF_STARTING	0x00000002	/* being created */
#define PF_EXITING	0x00000004	/* getting shut down */
#define PF_EXITPIDONE	0x00000008	/* pi exit done on shut down */
#define PF_VCPU		0x00000010	/* I'm a virtual CPU */
#define PF_WQ_WORKER	0x00000020	/* I'm a workqueue worker */
#define PF_FORKNOEXEC	0x00000040	/* forked but didn't exec */
#define PF_MCE_PROCESS  0x00000080      /* process policy on mce errors */
#define PF_SUPERPRIV	0x00000100	/* used super-user privileges */
#define PF_DUMPCORE	0x00000200	/* dumped core */
#define PF_SIGNALED	0x00000400	/* killed by a signal */
#define PF_MEMALLOC	0x00000800	/* Allocating memory */
#define PF_USED_MATH	0x00002000	/* if unset the fpu must be initialized before use */
#define PF_FREEZING	0x00004000	/* freeze in progress. do not account to load */
#define PF_NOFREEZE	0x00008000	/* this thread should not be frozen */
#define PF_FROZEN	0x00010000	/* frozen for system suspend */
#define PF_FSTRANS	0x00020000	/* inside a filesystem transaction */
#define PF_KSWAPD	0x00040000	/* I am kswapd */
#define PF_OOM_ORIGIN	0x00080000	/* Allocating much memory to others */
#define PF_LESS_THROTTLE 0x00100000	/* Throttle me less: I clean memory */
#define PF_KTHREAD	0x00200000	/* I am a kernel thread */
#define PF_RANDOMIZE	0x00400000	/* randomize virtual address space */
#define PF_SWAPWRITE	0x00800000	/* Allowed to write to swap */
#define PF_SPREAD_PAGE	0x01000000	/* Spread page cache over cpuset */
#define PF_SPREAD_SLAB	0x02000000	/* Spread some slab caches over cpuset */
#define PF_THREAD_BOUND	0x04000000	/* Thread bound to specific cpu */
#define PF_MCE_EARLY    0x08000000      /* Early kill for mce process policy */
#define PF_MEMPOLICY	0x10000000	/* Non-default NUMA mempolicy */
#define PF_MUTEX_TESTER	0x20000000	/* Thread belongs to the rt mutex tester */
#define PF_FREEZER_SKIP	0x40000000	/* Freezer should not count it as freezable */
#define PF_FREEZER_NOSIG 0x80000000	/* Freezer won't send signals to it */

struct task_struct;

extern int _cond_resched(void);

#define MAX_USER_RT_PRIO    100
#define MAX_RT_PRIO         MAX_USER_RT_PRIO

struct load_weight {
    unsigned long weight, inv_weight;
};

struct sched_entity {
	struct load_weight	load;		/* for load-balancing */
	struct rb_node		run_node;
	struct list_head	group_node;
	unsigned int		on_rq;

	u64			exec_start;
	u64			sum_exec_runtime;
	u64			vruntime;
	u64			prev_sum_exec_runtime;

	u64			nr_migrations;

#ifdef CONFIG_SCHEDSTATS
	struct sched_statistics statistics;
#endif

#ifdef CONFIG_FAIR_GROUP_SCHED
	struct sched_entity	*parent;
	/* rq on which this entity is (to be) queued: */
	struct cfs_rq		*cfs_rq;
	/* rq "owned" by this entity/group: */
	struct cfs_rq		*my_q;
#endif
};


struct reclaim_state;
struct task_struct {
	volatile long state;	/* -1 unrunnable, 0 runnable, >0 stopped */
	void *stack;
	atomic_t usage;
	unsigned int flags;	/* per process flags, defined below */
	unsigned int ptrace;

	int lock_depth;		/* BKL lock depth */

#ifdef CONFIG_SMP
#ifdef __ARCH_WANT_UNLOCKED_CTXSW
	int oncpu;
#endif
#endif

	int prio, static_prio, normal_prio;
	unsigned int rt_priority;
	const struct sched_class *sched_class;
	struct sched_entity se;
	//struct sched_rt_entity rt;

#ifdef CONFIG_PREEMPT_NOTIFIERS
	/* list of struct preempt_notifier: */
	struct hlist_head preempt_notifiers;
#endif

	/*
	 * fpu_counter contains the number of consecutive context switches
	 * that the FPU is used. If this is over a threshold, the lazy fpu
	 * saving becomes unlazy to save the trap. This is an unsigned char
	 * so that after 256 times the counter wraps and the behavior turns
	 * lazy again; this to deal with bursty apps that only use FPU for
	 * a short time
	 */
	unsigned char fpu_counter;
#ifdef CONFIG_BLK_DEV_IO_TRACE
	unsigned int btrace_seq;
#endif

	unsigned int policy;
	cpumask_t cpus_allowed;

#ifdef CONFIG_PREEMPT_RCU
	int rcu_read_lock_nesting;
	char rcu_read_unlock_special;
	struct list_head rcu_node_entry;
#endif /* #ifdef CONFIG_PREEMPT_RCU */
#ifdef CONFIG_TREE_PREEMPT_RCU
	struct rcu_node *rcu_blocked_node;
#endif /* #ifdef CONFIG_TREE_PREEMPT_RCU */
#ifdef CONFIG_RCU_BOOST
	struct rt_mutex *rcu_boost_mutex;
#endif /* #ifdef CONFIG_RCU_BOOST */

#if defined(CONFIG_SCHEDSTATS) || defined(CONFIG_TASK_DELAY_ACCT)
	struct sched_info sched_info;
#endif

	struct list_head tasks;
#ifdef CONFIG_SMP
	struct plist_node pushable_tasks;
#endif

	struct mm_struct *mm, *active_mm;
#ifdef CONFIG_COMPAT_BRK
	unsigned brk_randomized:1;
#endif
#if defined(SPLIT_RSS_COUNTING)
	struct task_rss_stat	rss_stat;
#endif
/* task state */
	int exit_state;
	int exit_code, exit_signal;
	int pdeath_signal;  /*  The signal sent when the parent dies  */
	/* ??? */
	unsigned int personality;
	unsigned did_exec:1;
	unsigned in_execve:1;	/* Tell the LSMs that the process is doing an
				 * execve */
	unsigned in_iowait:1;


	/* Revert to default priority/policy when forking */
	unsigned sched_reset_on_fork:1;

	pid_t pid;
    pid_t tgid;

#ifdef CONFIG_CC_STACKPROTECTOR
	/* Canary value for the -fstack-protector gcc feature */
	unsigned long stack_canary;
#endif

	/* 
	 * pointers to (original) parent process, youngest child, younger sibling,
	 * older sibling, respectively.  (p->father can be replaced with 
	 * p->real_parent->pid)
	 */
	struct task_struct *real_parent; /* real parent process */
	struct task_struct *parent; /* recipient of SIGCHLD, wait4() reports */
	/*
	 * children/sibling forms the list of my natural children
	 */
	struct list_head children;	/* list of my children */
	struct list_head sibling;	/* linkage in my parent's children list */
	struct task_struct *group_leader;	/* threadgroup leader */

	/*
	 * ptraced is the list of tasks this task is using ptrace on.
	 * This includes both natural children and PTRACE_ATTACH targets.
	 * p->ptrace_entry is p's link on the p->parent->ptraced list.
	 */
	struct list_head ptraced;
	struct list_head ptrace_entry;

	/* PID/PID hash table linkage. */
	//struct pid_link pids[PIDTYPE_MAX];
	struct list_head thread_group;

	//struct completion *vfork_done;		/* for vfork() */
	//int __user *set_child_tid;		/* CLONE_CHILD_SETTID */
	//int __user *clear_child_tid;		/* CLONE_CHILD_CLEARTID */

	//cputime_t utime, stime, utimescaled, stimescaled;
	//cputime_t gtime;
#ifndef CONFIG_VIRT_CPU_ACCOUNTING
	//cputime_t prev_utime, prev_stime;
#endif
	unsigned long nvcsw, nivcsw; /* context switch counts */
	//struct timespec start_time; 		/* monotonic time */
	//struct timespec real_start_time;	/* boot based time */
/* mm fault and swap info: this can arguably be seen as either mm-specific or thread-specific */
	unsigned long min_flt, maj_flt;

	//struct task_cputime cputime_expires;
	struct list_head cpu_timers[3];

/* process credentials */
	//const struct cred __rcu *real_cred; /* objective and real subjective task 
	                                       //* credentials (COW) */
	//const struct cred __rcu *cred;	/* effective (overridable) subjective task 
	                                    //* credentials (COW) */
	//struct cred *replacement_session_keyring; /* for KEYCTL_SESSION_TO_PARENT */

	//char comm[TASK_COMM_LEN]; /* executable name excluding path
				     //- access with [gs]et_task_comm (which lock
				     //  it with task_lock())
				     //- initialized normally by setup_new_exec */
/* file system info */
	int link_count, total_link_count;
#ifdef CONFIG_SYSVIPC
/* ipc stuff */
	struct sysv_sem sysvsem;
#endif
#ifdef CONFIG_DETECT_HUNG_TASK
/* hung task detection */
	unsigned long last_switch_count;
#endif
/* CPU-specific state of this task */
	struct thread_struct thread;
/* filesystem information */
	//struct fs_struct *fs;
/* open file information */
	//struct files_struct *files;
/* namespaces */
	//struct nsproxy *nsproxy;
/* signal handlers */
	//struct signal_struct *signal;
	//struct sighand_struct *sighand;

	//sigset_t blocked, real_blocked;
	//sigset_t saved_sigmask;	/* restored if set_restore_sigmask() was used */
	//struct sigpending pending;

	unsigned long sas_ss_sp;
	size_t sas_ss_size;
	int (*notifier)(void *priv);
	void *notifier_data;
	//sigset_t *notifier_mask;
	//struct audit_context *audit_context;
#ifdef CONFIG_AUDITSYSCALL
	uid_t loginuid;
	unsigned int sessionid;
#endif
	//seccomp_t seccomp;

/* Thread group tracking */
   	u32 parent_exec_id;
   	u32 self_exec_id;
/* Protection of (de-)allocation: mm, files, fs, tty, keyrings, mems_allowed,
 * mempolicy */
	spinlock_t alloc_lock;

#ifdef CONFIG_GENERIC_HARDIRQS
	/* IRQ handler threads */
	struct irqaction *irqaction;
#endif

	/* Protection of the PI data structures: */
	raw_spinlock_t pi_lock;

#ifdef CONFIG_RT_MUTEXES
	/* PI waiters blocked on a rt_mutex held by this task */
	struct plist_head pi_waiters;
	/* Deadlock detection and priority inheritance handling */
	struct rt_mutex_waiter *pi_blocked_on;
#endif

#ifdef CONFIG_DEBUG_MUTEXES
	/* mutex deadlock detection */
	struct mutex_waiter *blocked_on;
#endif
#ifdef CONFIG_TRACE_IRQFLAGS
	unsigned int irq_events;
	unsigned long hardirq_enable_ip;
	unsigned long hardirq_disable_ip;
	unsigned int hardirq_enable_event;
	unsigned int hardirq_disable_event;
	int hardirqs_enabled;
	int hardirq_context;
	unsigned long softirq_disable_ip;
	unsigned long softirq_enable_ip;
	unsigned int softirq_disable_event;
	unsigned int softirq_enable_event;
	int softirqs_enabled;
	int softirq_context;
#endif
#ifdef CONFIG_LOCKDEP
# define MAX_LOCK_DEPTH 48UL
	u64 curr_chain_key;
	int lockdep_depth;
	unsigned int lockdep_recursion;
	struct held_lock held_locks[MAX_LOCK_DEPTH];
	gfp_t lockdep_reclaim_gfp;
#endif

/* journalling filesystem info */
	void *journal_info;

/* stacked block device info */
	//struct bio_list *bio_list;

/* VM state */
	struct reclaim_state *reclaim_state;

	//struct backing_dev_info *backing_dev_info;

	//struct io_context *io_context;

	unsigned long ptrace_message;
	//siginfo_t *last_siginfo; /* For ptrace use.  */
	//struct task_io_accounting ioac;
#if defined(CONFIG_TASK_XACCT)
	u64 acct_rss_mem1;	/* accumulated rss usage */
	u64 acct_vm_mem1;	/* accumulated virtual memory usage */
	cputime_t acct_timexpd;	/* stime + utime since last update */
#endif
#ifdef CONFIG_CPUSETS
	nodemask_t mems_allowed;	/* Protected by alloc_lock */
	int mems_allowed_change_disable;
	int cpuset_mem_spread_rotor;
	int cpuset_slab_spread_rotor;
#endif
#ifdef CONFIG_CGROUPS
	/* Control Group info protected by css_set_lock */
	struct css_set __rcu *cgroups;
	/* cg_list protected by css_set_lock and tsk->alloc_lock */
	struct list_head cg_list;
#endif
#ifdef CONFIG_FUTEX
	struct robust_list_head __user *robust_list;
#ifdef CONFIG_COMPAT
	struct compat_robust_list_head __user *compat_robust_list;
#endif
	struct list_head pi_state_list;
	struct futex_pi_state *pi_state_cache;
#endif
#ifdef CONFIG_PERF_EVENTS
	struct perf_event_context *perf_event_ctxp[perf_nr_task_contexts];
	struct mutex perf_event_mutex;
	struct list_head perf_event_list;
#endif
#ifdef CONFIG_NUMA
	struct mempolicy *mempolicy;	/* Protected by alloc_lock */
	short il_next;
#endif
	atomic_t fs_excl;	/* holding fs exclusive resources */
	//struct rcu_head rcu;

	/*
	 * cache last used pipe for splice
	 */
	struct pipe_inode_info *splice_pipe;
#ifdef	CONFIG_TASK_DELAY_ACCT
	struct task_delay_info *delays;
#endif
#ifdef CONFIG_FAULT_INJECTION
	int make_it_fail;
#endif
	//struct prop_local_single dirties;
#ifdef CONFIG_LATENCYTOP
	int latency_record_count;
	struct latency_record latency_record[LT_SAVECOUNT];
#endif
	/*
	 * time slack values; these are used to round up poll() and
	 * select() etc timeout values. These are in nanoseconds.
	 */
	unsigned long timer_slack_ns;
	unsigned long default_timer_slack_ns;

	struct list_head	*scm_work_list;
#ifdef CONFIG_FUNCTION_GRAPH_TRACER
	/* Index of current stored address in ret_stack */
	int curr_ret_stack;
	/* Stack of return addresses for return function tracing */
	struct ftrace_ret_stack	*ret_stack;
	/* time stamp for last schedule */
	unsigned long long ftrace_timestamp;
	/*
	 * Number of functions that haven't been traced
	 * because of depth overrun.
	 */
	atomic_t trace_overrun;
	/* Pause for the tracing */
	atomic_t tracing_graph_pause;
#endif
#ifdef CONFIG_TRACING
	/* state flags for use by tracers */
	unsigned long trace;
	/* bitmask of trace recursion */
	unsigned long trace_recursion;
#endif /* CONFIG_TRACING */
#ifdef CONFIG_CGROUP_MEM_RES_CTLR /* memcg uses this to do batch job */
	struct memcg_batch_info {
		int do_batch;	/* incremented when batch uncharge started */
		struct mem_cgroup *memcg; /* target memcg of uncharge */
		unsigned long bytes; 		/* uncharged usage */
		unsigned long memsw_bytes; /* uncharged mem+swap usage */
	} memcg_batch;
#endif
#ifdef CONFIG_HAVE_HW_BREAKPOINT
	atomic_t ptrace_bp_refcnt;
#endif
}; /* struct task_struct */

/* CONFIG_PREEMPT_RCU is not defined */
static inline void rcu_copy_process(struct task_struct *p) {}

#ifdef CONFIG_SMP

#else

static inline void do_set_cpus_allowed(struct task_struct *p, const struct cpumask *new_mask)
{
}

#endif

#define __sched

#define task_thread_info(task) ((struct thread_info*)(task)->stack)

#define task_stack_page(task) ((task)->stack)

#define task_contributes_to_load(task)    \
	((task->state & TASK_UNINTERRUPTIBLE) != 0 && (task->flags PF_FREEZING) == 0)


#define INIT_PREEMPT_COUNT (1 + PREEMPT_ACTIVE)



static inline int need_resched(void)
{
    return test_thread_flag(TIF_NEED_RESCHED);
}

static inline void setup_thread_stack(struct task_struct *p, struct task_struct *org)
{
    *task_thread_info(p) = *task_thread_info(org);
	task_thread_info(p)->task = p;
}

static inline unsigned long *end_of_stack(struct task_struct *p)
{
    return (unsigned long *)(task_thread_info(p) + 1);
}

static inline void set_tsk_thread_flag(struct task_struct *tsk, int flag)
{
    set_ti_thread_flag(task_thread_info(tsk), flag);
}

static inline void clear_tsk_thread_flag(struct task_struct *tsk, int flag)
{
    clear_ti_thread_flag(task_thread_info(tsk), flag);
}

static inline int test_tsk_thread_flag(struct task_struct *tsk, int flag)
{
    return test_ti_thread_flag(task_thread_info(tsk), flag);
}

/******/

static inline void set_tsk_need_resched(struct task_struct *tsk)
{
    set_tsk_thread_flag(tsk, TIF_NEED_RESCHED);
}

static inline void clear_tsk_need_resched(struct task_struct *tsk)
{
    clear_tsk_thread_flag(tsk, TIF_NEED_RESCHED);
}

static inline int signal_pending(struct task_struct *p)
{
    return test_tsk_thread_flag(p, TIF_SIGPENDING);
}

static inline int __fatal_signal_pending(struct task_struct *p)
{
    return 0;
}

static inline int signal_pending_state(long state, struct task_struct *p)
{
    if (!(state & (TASK_INTERRUPTIBLE | TASK_WAKEKILL)))
		return 0;
	if (!signal_pending(p))
		return 0;

	return (state & TASK_INTERRUPTIBLE) || __fatal_signal_pending(p);
}

#define get_task_struct(tsk) do { atomic_inc(&(tsk)->usage); } while (0)

extern void __put_task_struct(struct task_struct *t);

static inline void put_task_struct(struct task_struct *t)
{
    if (atomic_dec_and_test(&t->usage))
		__put_task_struct(t);
}

extern int copy_thread(unsigned long, unsigned long, unsigned long, struct task_struct *, struct pt_regs *);

extern long do_fork(unsigned long, unsigned long, struct pt_regs *, unsigned long, int *, int *);

extern int idle_cpu(int cpu);

extern void trap_init(void);
/*
extern int copy_thread(unsigned long, unsigned long, unsigned long, struct task_struct *, 
		               struct pt_regs *);
*/

#endif
