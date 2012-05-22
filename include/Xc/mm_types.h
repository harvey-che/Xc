#ifndef _XC_MM_TYPES_H
#define _XC_MM_TYPES_H

#include <Xc/types.h>
#include <Xc/atomic.h>
#include <Xc/spinlock.h>
#include <Xc/rbtree.h>
#include <Xc/cpumask.h>

/* asm/mmu.h */
typedef struct {
    void *ldt;
	int size;
	/* struct mutex lock; */
	void *vdso;
} mm_context_t;
/* end -- asm/mmu.h */

struct address_space;
struct kmem_cache;

struct page {
    unsigned long flags;
	struct address_space *mapping;

	struct {
		union {
			pgoff_t index;
			void *freelist;
		};
        union {
            unsigned long counters;
			struct {
                union {
                    atomic_t _mapcount;
					struct {
                        unsigned inuse:16;
						unsigned objects:15;
						unsigned frozen:1;
					};
				};
				atomic_t _count;
			};
		};
	};

	struct list_head lru;

	union {
        unsigned long private;
		struct kmem_cache *slab;
		struct page *first_page;
	};
};

struct mm_struct {
	//struct vm_area_struct * mmap;		/* list of VMAs */
	struct rb_root mm_rb;
	//struct vm_area_struct * mmap_cache;	/* last find_vma result */
#ifdef CONFIG_MMU
	//unsigned long (*get_unmapped_area) (struct file *filp,
	//			unsigned long addr, unsigned long len,
	//			unsigned long pgoff, unsigned long flags);
	//void (*unmap_area) (struct mm_struct *mm, unsigned long addr);
#endif
	unsigned long mmap_base;		/* base of mmap area */
	unsigned long task_size;		/* size of task vm space */
	unsigned long cached_hole_size; 	/* if non-zero, the largest hole below free_area_cache */
	unsigned long free_area_cache;		/* first hole of size cached_hole_size or larger */
	pgd_t * pgd;
	atomic_t mm_users;			/* How many users with user space? */
	atomic_t mm_count;			/* How many references to "struct mm_struct" (users count as 1) */
	int map_count;				/* number of VMAs */
	//struct rw_semaphore mmap_sem;
	spinlock_t page_table_lock;		/* Protects page tables and some counters */

	struct list_head mmlist;		/* List of maybe swapped mm's.	These are globally strung
						 * together off init_mm.mmlist, and are protected
						 * by mmlist_lock
						 */


	unsigned long hiwater_rss;	/* High-watermark of RSS usage */
	unsigned long hiwater_vm;	/* High-water virtual memory usage */

	unsigned long total_vm, locked_vm, shared_vm, exec_vm;
	unsigned long stack_vm, reserved_vm, def_flags, nr_ptes;
	unsigned long start_code, end_code, start_data, end_data;
	unsigned long start_brk, brk, start_stack;
	unsigned long arg_start, arg_end, env_start, env_end;

	//unsigned long saved_auxv[AT_VECTOR_SIZE]; /* for /proc/PID/auxv */

	/*
	 * Special counters, in some configurations protected by the
	 * page_table_lock, in other configurations by being atomic.
	 */
	//struct mm_rss_stat rss_stat;

	//struct linux_binfmt *binfmt;

	cpumask_t cpu_vm_mask;

	/* Architecture-specific MM context */
	mm_context_t context;

	/* Swap token stuff */
	/*
	 * Last value of global fault stamp as seen by this process.
	 * In other words, this value gives an indication of how long
	 * it has been since this task got the token.
	 * Look at mm/thrash.c
	 */
	unsigned int faultstamp;
	unsigned int token_priority;
	unsigned int last_interval;

	unsigned long flags; /* Must use atomic bitops to access the bits */

	//struct core_state *core_state; /* coredumping support */
#ifdef CONFIG_AIO
	spinlock_t		ioctx_lock;
	struct hlist_head	ioctx_list;
#endif
#ifdef CONFIG_MM_OWNER
	/*
	 * "owner" points to a task that is regarded as the canonical
	 * user/owner of this mm. All of the following must be true in
	 * order for it to be changed:
	 *
	 * current == mm->owner
	 * current->mm != mm
	 * new_owner->mm == mm
	 * new_owner->alloc_lock is held
	 */
	struct task_struct __rcu *owner;
#endif

#ifdef CONFIG_PROC_FS
	/* store ref to file /proc/<pid>/exe symlink points to */
	struct file *exe_file;
	unsigned long num_exe_file_vmas;
#endif
#ifdef CONFIG_MMU_NOTIFIER
	struct mmu_notifier_mm *mmu_notifier_mm;
#endif
#ifdef CONFIG_TRANSPARENT_HUGEPAGE
	pgtable_t pmd_huge_pte; /* protected by page_table_lock */
#endif
	/* How many tasks sharing this mm are OOM_DISABLE */
	atomic_t oom_disable_count;
};

#endif
