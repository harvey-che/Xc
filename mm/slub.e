# 1 "slub.c"
# 1 "/home/harvey/myos/mm//"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "slub.c"
# 1 "../include/Xc/slab.h" 1



# 1 "../include/Xc/gfp.h" 1



# 1 "../include/Xc/types.h" 1






typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long s64;
typedef unsigned long long u64;

typedef int size_t;




typedef signed char __s8;
typedef unsigned char __u8;

typedef signed short __s16;
typedef unsigned short __u16;

typedef signed int __s32;
typedef unsigned int __u32;

typedef signed long long __s64;
typedef unsigned long long __u64;

typedef int size_t;

typedef u32 phys_addr_t;

typedef phys_addr_t resource_size_t;







typedef u32 phys_addr_t;

typedef phys_addr_t resource_size_t;

typedef struct {
    int counter;
} atomic_t;

struct list_head {
    struct list_head *next, *prev;
};

struct hlist_head {
    struct hlist_node *first;
};

struct hlist_node {
    struct hlist_node *next, **prev;
};

typedef int bool;



typedef unsigned gfp_t;
# 5 "../include/Xc/gfp.h" 2
# 1 "../include/Xc/mmzone.h" 1



# 1 "../include/Xc/list.h" 1
# 19 "../include/Xc/list.h"
static inline void INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}

static inline void __list_del(struct list_head *prev, struct list_head *next)
{
    next->prev = prev;
 prev->next = next;
}

static inline void __list_del_entry(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
}

static inline void list_del(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
 entry->next = ((void*)0x00100100);
 entry->prev = ((void*)0x00200200);
}

static inline void __list_add(struct list_head *new, struct list_head *prev, struct list_head *next)
{
    next->prev = new;
 new->next = next;
 new->prev = prev;
 prev->next = new;
}

static inline void list_add(struct list_head *new, struct list_head *head)
{
    __list_add(new, head, head->next);
}

static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
    __list_add(new, head->prev, head);
}
# 5 "../include/Xc/mmzone.h" 2
# 1 "../include/Xc/spinlock.h" 1



# 1 "../include/Xc/irqflags.h" 1



# 1 "../include/Xc/typecheck.h" 1
# 5 "../include/Xc/irqflags.h" 2







static inline unsigned long native_save_fl(void)
{
    unsigned long flags;





 asm volatile ("# __raw_save_flags \n\t"
         "pushf; pop %0"
      : "=rm"(flags)
      : : "memory");
 return flags;
}

static inline void native_restore_fl(unsigned long flags)
{
    asm volatile ("push %0; popf"
         : : "g"(flags)
      : "memory", "cc");
}

static inline void native_irq_disable(void)
{
    asm volatile ("cli": : : "memory");
}

static inline void native_irq_enable(void)
{
    asm volatile ("sti": : : "memory");
}

static inline unsigned long arch_local_save_flags(void)
{
    return native_save_fl();
}

static inline void arch_local_irq_disable(void)
{
    native_irq_disable();
}

static inline void arch_local_irq_enable(void)
{
    native_irq_enable();
}

static inline unsigned long arch_local_irq_save(void)
{
    unsigned long flags = arch_local_save_flags();
 arch_local_irq_disable();
 return flags;
}

static inline void arch_local_irq_restore(unsigned long flags)
{
    native_restore_fl(flags);
}
# 5 "../include/Xc/spinlock.h" 2
# 1 "../include/Xc/spinlock_types.h" 1





typedef struct {
    volatile unsigned int slock;
} arch_spinlock_t;




typedef struct raw_spinlock {
    arch_spinlock_t raw_lock;

 unsigned int magic, owner_cpu;
 void *owner;
} raw_spinlock_t;

typedef struct spinlock {
    union {
        struct raw_spinlock rlock;
 };
} spinlock_t;
# 6 "../include/Xc/spinlock.h" 2
# 1 "../include/Xc/compiler-gcc.h" 1
# 7 "../include/Xc/spinlock.h" 2
# 1 "../include/Xc/preempt.h" 1



# 1 "../include/Xc/thread_info.h" 1
# 10 "../include/Xc/thread_info.h"
# 1 "../include/types.h" 1






typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long s64;
typedef unsigned long long u64;





typedef signed char __s8;
typedef unsigned char __u8;

typedef signed short __s16;
typedef unsigned short __u16;

typedef signed int __s32;
typedef unsigned int __u32;

typedef signed long long __s64;
typedef unsigned long long __u64;

typedef int size_t;

typedef u32 phys_addr_t;

typedef phys_addr_t resource_size_t;
# 11 "../include/Xc/thread_info.h" 2
# 1 "../include/asm/page_types.h" 1



# 1 "../include/Xc/const.h" 1
# 5 "../include/asm/page_types.h" 2
# 12 "../include/Xc/thread_info.h" 2
# 1 "../include/Xc/bitops.h" 1
# 9 "../include/Xc/bitops.h"
# 1 "../include/asm/alternative.h" 1
# 10 "../include/Xc/bitops.h" 2
# 26 "../include/Xc/bitops.h"
static void set_bit(unsigned int nr, volatile unsigned long *addr)
{
    if ((__builtin_constant_p(nr))) {
        asm volatile("" "orb %1, %0"
    : "+m" (*(volatile long *)((void *)(addr) + ((nr) >> 3)))
    : "iq"((u8)(1 << ((nr) & 7)))
    : "memory");
 } else {
        asm volatile("" "bts %1, %0"
    : "+m" (*(volatile long *)(addr)) : "Ir"(nr) : "memory");
 }
}

static inline void __set_bit(int nr, volatile unsigned long *addr)
{
    asm volatile("bts %1, %0" : "+m" (*(volatile long *)(addr)) : "Ir"(nr) : "memroy");
}

static inline void clear_bit(int nr, volatile unsigned long *addr)
{
    if ((__builtin_constant_p(nr))) {
        asm volatile("" "andb %1, %0"
    : "+m" (*(volatile long *)((void *)(addr) + ((nr) >> 3)))
    : "iq"((u8)~(1 << ((nr) & 7))));
 } else {
        asm volatile("" "btr %1, %0"
    : "+m" (*(volatile long *)(addr))
    : "Ir" (nr));
 }
}

static inline void clear_bit_unlock(unsigned nr, volatile unsigned long *addr)
{
    __asm__ __volatile__("": : :"memory");
 clear_bit(nr, addr);
}

static inline void __clear_bit(int nr, volatile unsigned long *addr)
{
    asm volatile("btr %1, %0" : "+m" (*(volatile long *)(addr)) : "Ir"(nr));
}

static inline void __clear_bit_unlock(unsigned nr, volatile unsigned long *addr)
{
    __asm__ __volatile__("": : :"memory");
 __clear_bit(nr, addr);
}
# 81 "../include/Xc/bitops.h"
static inline int constant_test_bit(unsigned int nr, const volatile unsigned long *addr)
{
    return ((1UL << (nr % 32)) & (addr[nr / 32])) != 0;
}

static inline int variable_test_bit(int nr, volatile const unsigned long *addr)
{
    int oldbit;
 asm volatile("bt %2, %1\n\t"
        "sbb %0, %0"
     : "=r"(oldbit)
     : "m"(*(unsigned long *)addr), "Ir"(nr));
 return oldbit;
}
# 212 "../include/Xc/bitops.h"
static inline int ffs(int x)
{
    int r;

 asm("bsfl %1, %0\n\t"
  "comvzl %2, %0"
  : "=r"(r) : "rm"(x), "r"(-1));

 return r + 1;
}

static inline int fls(int x)
{
    int r;

 asm("bsrl %1, %0\n\t"
  "cmovzl %2, %0"
  : "=&r"(r) : "rm"(x), "rm"(-1));

 return r + 1;
}




static int fls64(__u64 x)
{
    __u32 h = x >> 32;
 if (h)
  return fls(h) + 32;
 return fls(x);
}



static inline unsigned fls_long(unsigned long l)
{

    if (sizeof(l) == 4)
  return fls(l);
 return fls64(l);

}
# 13 "../include/Xc/thread_info.h" 2





struct task_struct;

struct thread_info {
    struct task_struct *task;
 __u32 flags;
 __u32 status;
 int preempt_count;
 unsigned long previous_esp;
 __u8 supervisor_stack[0];
};
# 39 "../include/Xc/thread_info.h"
register unsigned long current_stack_pointer asm("esp");

static inline struct thread_info *current_thread_info(void)
{
    return (struct thread_info*)(current_stack_pointer & ~((((1UL) << 12) << 1) - 1));
}

static inline int test_ti_thread_flag(struct thread_info *ti, int flag)
{
    return (__builtin_constant_p((flag)) ? constant_test_bit((flag), ((unsigned long *)&ti->flags)) : variable_test_bit((flag), ((unsigned long *)&ti->flags)));
}
# 5 "../include/Xc/preempt.h" 2

void preempt_schedule(void);
# 8 "../include/Xc/spinlock.h" 2
# 33 "../include/Xc/spinlock.h"
static inline raw_spinlock_t *spinlock_check(spinlock_t *lock)
{
    return &lock->rlock;
}
# 62 "../include/Xc/spinlock.h"
static inline void spin_lock(spinlock_t *lock)
{
    do { do { do { (current_thread_info()->preempt_count) += (1); } while (0); __asm__ __volatile__("": : :"memory"); } while (0); (void)(&lock->rlock); } while (0);
}

static inline void spin_unlock(spinlock_t *lock)
{
    do { do { do { __asm__ __volatile__("": : :"memory"); do { (current_thread_info()->preempt_count) -= (1); } while (0); } while (0); __asm__ __volatile__("": : :"memory"); do { if (test_ti_thread_flag(current_thread_info(), 3)) preempt_schedule(); } while (0); } while (0); (void)(&lock->rlock); } while (0);
}






static inline void spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags)
{
    do { ({ unsigned long __dummy; typeof(flags) __dummy2; (void)(&__dummy == &__dummy2); 1; }); do { do { do { ({ unsigned long __dummy; typeof(flags) __dummy2; (void)(&__dummy == &__dummy2); 1; }); arch_local_irq_restore(flags); } while (0); } while (0); do { do { do { __asm__ __volatile__("": : :"memory"); do { (current_thread_info()->preempt_count) -= (1); } while (0); } while (0); __asm__ __volatile__("": : :"memory"); do { if (test_ti_thread_flag(current_thread_info(), 3)) preempt_schedule(); } while (0); } while (0); (void)(&lock->rlock); } while (0); } while (0); } while (0);
}

# 1 "../include/Xc/atomic.h" 1




# 1 "../include/Xc/spinlock.h" 1
# 6 "../include/Xc/atomic.h" 2
# 27 "../include/Xc/atomic.h"
static inline void atomic_set(atomic_t *v, int i)
{
    v->counter = i;
}

static inline void atomic_add(int i, atomic_t *v)
{
    asm volatile("" "addl %1, %0"
   : "+m"(v->counter)
   : "ir"(i));
}



typedef atomic_t atomic_long_t;

static inline void atomic_long_add(long i, atomic_long_t *l)
{
    atomic_t *v = (atomic_t)l;
 atomic_add(i, v);
}
# 83 "../include/Xc/spinlock.h" 2
# 6 "../include/Xc/mmzone.h" 2
# 1 "../include/Xc/wait.h" 1




# 1 "../include/Xc/lockdep.h" 1




struct lock_class_key {};
# 6 "../include/Xc/wait.h" 2


struct __wait_queue_head {
    spinlock_t lock;
 struct list_head task_list;
};

typedef struct __wait_queue_head wait_queue_head_t;


extern void __init_waitqueue_head(wait_queue_head_t *q, struct lock_class_key*);
# 7 "../include/Xc/mmzone.h" 2
# 1 "../include/asm/atomic.h" 1
# 8 "../include/Xc/mmzone.h" 2


# 1 "../include/Xc/kernel.h" 1





# 1 "../include/stddef.h" 1
# 13 "../include/stddef.h"
enum {
    false = 0,
 true = 1
};
# 7 "../include/Xc/kernel.h" 2

int printk(const char *format, ...);
void panic(const char *format, ...);
# 43 "../include/Xc/kernel.h"
extern void sort(void *base, size_t num, size_t size, int (*cmp_func)(const void *, const void *),
  void (*swap_func)(void *, void *, int size));
# 54 "../include/Xc/kernel.h"
extern int ___ilog2_NaN(void);

static int __ilog2_u32(u32 n)
{
    return fls(n) - 1;
}

static int __ilog2_u64(u64 n)
{
    return fls64(n) - 1;
}
# 141 "../include/Xc/kernel.h"
static inline unsigned long __roundup_pow_of_two(unsigned long n)
{
    return 1UL << fls_long(n - 1);
}
# 159 "../include/Xc/kernel.h"
static inline int get_order(unsigned long size)
{
    int order;
 size = (size - 1) >> (12 - 1);
 order = -1;
 do {
  size >>= 1;
  order++;
 } while (size);
 return order;
}
# 11 "../include/Xc/mmzone.h" 2




static inline int numa_node_id(void)
{
    return 0;
}





enum pageblock_bits {
    PB_migrate,
 PB_migrate_end = PB_migrate + 3 -1,
 NR_PAGEBLOCK_BITS
};

struct node_active_region {
    unsigned long start_pfn;
 unsigned long end_pfn;
 int nid;
};

enum zone_type {
    ZONE_DMA,
 ZONE_NORMAL,
 ZONE_MOVABLE,
 ZONE_HIGHMEM,
 __MAX_NR_ZONES
};





enum lru_list {
    LRU_INACTIVE_ANON = 0,
 LRU_ACTIVE_ANON = 0 + 1,
 LRU_INACTIVE_FILE = 0 + 2,
 LRU_ACTIVE_FILE = 0 + 2 +1,
 LRU_UNEVICTABLE,
 NR_LRU_LISTS
};
# 66 "../include/Xc/mmzone.h"
struct free_area {
    struct list_head free_list[5];
 unsigned long nr_free;
};
# 96 "../include/Xc/mmzone.h"
struct zone_reclaim_stat {
    unsigned long recent_rotated[2];
 unsigned long recent_scanned[2];
};

enum zone_stat_item {

 NR_FREE_PAGES,
 NR_LRU_BASE,
 NR_INACTIVE_ANON = NR_LRU_BASE,
 NR_ACTIVE_ANON,
 NR_INACTIVE_FILE,
 NR_ACTIVE_FILE,
 NR_UNEVICTABLE,
 NR_MLOCK,
 NR_ANON_PAGES,
 NR_FILE_MAPPED,

 NR_FILE_PAGES,
 NR_FILE_DIRTY,
 NR_WRITEBACK,
 NR_SLAB_RECLAIMABLE,
 NR_SLAB_UNRECLAIMABLE,
 NR_PAGETABLE,
 NR_KERNEL_STACK,

 NR_UNSTABLE_NFS,
 NR_BOUNCE,
 NR_VMSCAN_WRITE,
 NR_WRITEBACK_TEMP,
 NR_ISOLATED_ANON,
 NR_ISOLATED_FILE,
 NR_SHMEM,
 NR_DIRTIED,
 NR_WRITTEN,
# 139 "../include/Xc/mmzone.h"
 NR_ANON_TRANSPARENT_HUGEPAGES,
 NR_VM_ZONE_STAT_ITEMS };
enum zone_watermarks {
    WMARK_MIN,
 WMARK_LOW,
 WMARK_HIGH,
 NR_WMARK
};

struct per_cpu_pages {
    int count;
 int high;
 int batch;

 struct list_head lists[3];
};

struct per_cpu_pageset {
    struct per_cpu_pages pcp;
};
struct page;
struct zone {
 unsigned long watermark[NR_WMARK];

 struct per_cpu_pageset *pageset;

 spinlock_t lock;
 struct free_area free_area[11];

 unsigned long *pageblock_flags;

 spinlock_t lru_lock;
 struct zone_lru {
        struct list_head list;
 } lru[NR_LRU_LISTS];

    struct zone_reclaim_stat reclaim_stat;

 unsigned long pages_scanned;
 unsigned long flags;

 atomic_long_t vm_stat[NR_VM_ZONE_STAT_ITEMS];

    wait_queue_head_t *wait_table;
 unsigned long wait_table_hash_nr_entries;
 unsigned long wait_table_bits;

    struct pglist_data *zone_pgdat;

 unsigned long zone_start_pfn;
 unsigned long spanned_pages;
 unsigned long present_pages;

 const char *name;
};

struct zonelist_cache {

};

struct zoneref {
    struct zone *zone;
 int zone_idx;
};


struct zonelist_cache;

struct zonelist {
    struct zonelist_cache *zlcache_ptr;
 struct zoneref _zonerefs[(4 * (1 << 0)) + 1];
};

struct task_struct;

typedef struct pglist_data {
    struct zone node_zones[4];
 struct zonelist node_zonelists[1];
 int nr_zones;

 struct page *node_mem_map;

 unsigned long node_start_pfn;
 unsigned long node_present_pages;
 unsigned long node_spanned_pages;

 int node_id;

 wait_queue_head_t kswapd_wait;
 struct task_struct *kswapd;
 int kswapd_max_order;
 enum zone_type classzone_idx;
} pg_data_t;

extern struct pglist_data contig_page_data;
extern struct page *mem_map;







static inline int populated_zone(struct zone *zone)
{
    return (!!zone->present_pages);
}

enum memmap_context {
    MEMMAP_EARLY,
 MEMMAP_HOTPLUG,
};

extern int page_group_by_mobility_disabled;

extern int init_currently_empty_zone(struct zone *zone, unsigned long start_pfn, unsigned long size,
  enum memmap_context context);



enum node_states {
    N_POSSIBLE,
 N_ONLINE,
 N_NORMAL_MEMORY,
 N_HIGH_MEMORY,
 N_CPU,
 NR_NODE_STATES
};



typedef struct { unsigned long bits[((((1 << 0)) + (8 * sizeof(long)) -1) / (8 * sizeof(long)))]; } nodemask_t;

static inline int node_state(int node, enum node_states state)
{
    return node == 0;
}

static void node_set_state(int node, enum node_states state)
{
}

static void node_clear_state(int node, enum node_states state)
{
}
# 293 "../include/Xc/mmzone.h"
extern int movable_zone;

static inline int zone_movable_is_highmem(void)
{
    return movable_zone == ZONE_HIGHMEM;
}

static inline int is_highmem_idx(enum zone_type idx)
{
    return (idx == ZONE_HIGHMEM ||
   (idx == ZONE_MOVABLE && zone_movable_is_highmem()));
}





static inline struct zone *zonelist_zone(struct zoneref *zoneref)
{
    return zoneref->zone;
}

static inline int zonelist_zone_idx(struct zoneref *zoneref)
{
    return zoneref->zone_idx;
}

struct zoneref *next_zones_zonelist(struct zoneref *z, enum zone_type highest_zoneidx, nodemask_t *nodes,
  struct zone **zone);

static inline struct zoneref *first_zones_zonelist(struct zonelist *zonelist,
                                             enum zone_type highest_zoneidx,
                                             nodemask_t *nodes, struct zone **zone)
{
    return next_zones_zonelist(zonelist->_zonerefs, highest_zoneidx, nodes, zone);
}
# 6 "../include/Xc/gfp.h" 2
# 75 "../include/Xc/gfp.h"
extern gfp_t gfp_allowed_mask;
# 93 "../include/Xc/gfp.h"
static inline enum zone_type gfp_zone(gfp_t flags)
{
    enum zone_type z;
 int bit = (int)(flags & (((gfp_t)0x01u) | ((gfp_t)0x02u) | ((gfp_t)0x04u) | ((gfp_t)0x08u)));

 z = (( (ZONE_NORMAL << 0 * 2) | (ZONE_DMA << 0x01u * 2) | (ZONE_HIGHMEM << 0x02u * 2) | (ZONE_NORMAL << 0x04u * 2) | (ZONE_NORMAL << 0x08u * 2) | (ZONE_DMA << (0x08u | 0x01u) * 2) | (ZONE_MOVABLE << (0x08u | 0x02u) * 2) | (ZONE_NORMAL << (0x08u | 0x04u) * 2) ) >> (bit * 2)) & ((1 << 2) - 1);
 return z;
}

static inline int allocflags_to_migratetype(gfp_t gfp_flags)
{
    if (page_group_by_mobility_disabled)
  return 0;

 return (((gfp_flags & ((gfp_t)0x08u)) != 0) << 1) | ((gfp_flags & ((gfp_t)0x80000u)) != 0);
}

extern void __free_pages(struct page *page, unsigned int order);
extern void free_pages(unsigned long addr, unsigned int order);




struct page * __alloc_pages_nodemask(gfp_t gfp_mask, unsigned int order, struct zonelist *zonelist,
  nodemask_t *nodemask);

static inline struct page *__alloc_pages(gfp_t gfp_mask, unsigned int order, struct zonelist *zonelist)
{
    return __alloc_pages_nodemask(gfp_mask, order, zonelist, ((void*)0));
}

static inline int gfp_zonelist(gfp_t flags)
{
    if (0 && (flags & ((gfp_t)0x40000u)))
  return 1;
 return 0;
}

static inline struct zonelist *node_zonelist(int nid, gfp_t flags)
{
    return (&contig_page_data)->node_zonelists + gfp_zonelist(flags);
}

static inline struct page *alloc_pages_node(int nid, gfp_t gfp_mask, unsigned int order)
{
    if (nid < 0)
  nid = numa_node_id();

 return __alloc_pages(gfp_mask, order, node_zonelist(nid, gfp_mask));
}




extern unsigned long __get_free_pages(gfp_t gfp_mask, unsigned int order);

static inline struct page *alloc_pages_exact_node(int nid, gfp_t gfp_mask, unsigned int order)
{




    return __alloc_pages(gfp_mask, order, node_zonelist(nid, gfp_mask));
}
# 5 "../include/Xc/slab.h" 2
# 28 "../include/Xc/slab.h"
# 1 "../include/Xc/slub_def.h" 1
# 9 "../include/Xc/slub_def.h"
struct kmem_cache_cpu {
    void **freelist;
 unsigned long tid;
 struct page *page;
 int node;
};

struct kmem_cache_node {
    spinlock_t list_lock;
 unsigned long nr_partial;
 struct list_head partial;
};

struct kmem_cache_order_objects {
    unsigned long x;
};

struct kmem_cache {
    struct kmem_cache_cpu *cpu_slab;
 unsigned long flags;
 unsigned long min_partial;
 int size;
 int objsize;
 int offset;
 struct kmem_cache_order_objects oo;

 struct kmem_cache_order_objects max;
 struct kmem_cache_order_objects min;
    gfp_t allocflags;
 int refcount;
 void (*ctor)(void *);
 int inuse;
 int align;
 int reserved;
 const char *name;
 struct list_head list;

 struct kmem_cache_node *node[(1 << 0)];
};
# 58 "../include/Xc/slub_def.h"
extern struct kmem_cache *kmalloc_caches[(12 + 2)];

static inline void *kmalloc_order(size_t size, gfp_t flags, unsigned int order)
{
    void *ret = (void *)__get_free_pages(flags | ((gfp_t)0x4000u), order);

 return ret;
}

static inline void *kmalloc_order_trace(size_t size, gfp_t flags, unsigned int order)
{
    return kmalloc_order(size, flags, order);
}

static inline void *kmalloc_large(size_t size, gfp_t flags)
{
    unsigned int order = get_order(size);
 return kmalloc_order_trace(size, flags, order);
}

static inline int kmalloc_index(size_t size)
{
    if (!size)
  return 0;
 if (size <= 8)
  return 3;
 if (8 <= 32 && size > 64 && size <= 96)
  return 1;
 if (8 <= 64 && size > 128 && size <= 192)
  return 2;

 if (size <= 8) return 3;
 if (size <= 16) return 4;
 if (size <= 32) return 5;
 if (size <= 64) return 6;
 if (size <= 128) return 7;
 if (size <= 256) return 8;
 if (size <= 512) return 9;
 if (size <= 1024) return 10;
 if (size <= 2 * 1024) return 11;
 if (size <= 4 * 1024) return 12;
 return -1;
}

static inline struct kmem_cache *kmalloc_slab(size_t size)
{
    int index = kmalloc_index(size);
 if (index == 0)
  return ((void*)0);
 return kmalloc_caches[index];
}

void *kmem_cache_alloc(struct kmem_cache *s, gfp_t flags);
void *__kmalloc(size_t size, gfp_t flags);

static inline void *kmem_cache_alloc_trace(struct kmem_cache *s, gfp_t gfpflags,
                                     size_t size)
{
    return kmem_cache_alloc(s, gfpflags);
}

static inline void *kmalloc(size_t size, gfp_t flags)
{
    if (__builtin_constant_p(size)) {
        if (size > (2 * ((1UL) << 12)))
   return kmalloc_large(size, flags);

  if (!(flags & ((gfp_t)0x01u))) {
            struct kmem_cache *s = kmalloc_slab(size);

   if (!s)
    return ((void *)16);
   return kmem_cache_alloc_trace(s, flags, size);
  }
 }
 return __kmalloc(size, flags);
}
# 29 "../include/Xc/slab.h" 2

static void *kmalloc_node(size_t size, gfp_t flags, int node)
{
    return ((void*)0);
}

int slab_is_available(void);

static inline void *kzalloc(size_t size, gfp_t flags)
{
    return kmalloc(size, flags | ((gfp_t)0x8000u));
}

static inline void *kzalloc_node(size_t size, gfp_t flags, int node)
{
    return kmalloc_node(size, flags | ((gfp_t)0x8000u), node);
}

static inline void *kmem_cache_alloc_node(struct kmem_cache *cachep, gfp_t flags, int nodes)
{
    return kmem_cache_alloc(cachep, flags);
}

void kmem_cache_init(void);
# 2 "slub.c" 2
# 1 "../include/Xc/threads.h" 1
# 3 "slub.c" 2


# 1 "../include/Xc/bit_spinlock.h" 1





static inline void bit_spin_lock(int bitnum, unsigned long *addr)
{
    do { do { (current_thread_info()->preempt_count) += (1); } while (0); __asm__ __volatile__("": : :"memory"); } while (0);
}


static inline void __bit_spin_unlock(int bitnum, unsigned long *addr)
{
    do { do { __asm__ __volatile__("": : :"memory"); do { (current_thread_info()->preempt_count) -= (1); } while (0); } while (0); __asm__ __volatile__("": : :"memory"); do { if (test_ti_thread_flag(current_thread_info(), 3)) preempt_schedule(); } while (0); } while (0);
}
# 6 "slub.c" 2
# 1 "../include/Xc/mm.h" 1




# 1 "../include/asm/page.h" 1
# 6 "../include/Xc/mm.h" 2
# 1 "../include/asm/pgtable_types.h" 1
# 7 "../include/Xc/mm.h" 2


# 1 "../include/Xc/mm_types.h" 1




struct address_space;
struct kmem_cache;

struct page {
    unsigned long flags;
 struct address_space *mapping;

 struct {
  union {
   unsigned long index;
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
# 10 "../include/Xc/mm.h" 2

# 1 "../include/Xc/page-flags.h" 1





enum pageflags {
 PG_locked,
 PG_error,
 PG_referenced,
 PG_uptodate,
 PG_dirty,
 PG_lru,
 PG_active,
 PG_slab,
 PG_owner_priv_1,
 PG_arch_1,
 PG_reserved,
 PG_private,
 PG_private_2,
 PG_writeback,

 PG_head,
 PG_tail,



 PG_swapcache,
 PG_mappedtodisk,
 PG_reclaim,
 PG_swapbacked,
 PG_unevictable,

 PG_mlocked,





 PG_hwpoison,




 __NR_PAGEFLAGS,


 PG_checked = PG_owner_priv_1,





 PG_fscache = PG_private_2,


 PG_pinned = PG_owner_priv_1,
 PG_savepinned = PG_dirty,


 PG_slob_free = PG_private,


 PG_slub_frozen = PG_active,
};
# 105 "../include/Xc/page-flags.h"
static inline int PageCompound(struct page *page)
{
    return page->flags & ((1L << PG_head) | (1L << PG_tail));
}

static inline int PageHead(const struct page *page) { return (__builtin_constant_p((PG_head)) ? constant_test_bit((PG_head), (&page->flags)) : variable_test_bit((PG_head), (&page->flags))); } static inline void __SetPageHead(struct page *page) { __set_bit(PG_head, &page->flags); } static inline void __ClearPageHead(struct page *page) { __clear_bit(PG_head, &page->flags); }

static inline void ClearPageHead(struct page *page) { clear_bit(PG_head, &page->flags); }

static inline int PageTail(const struct page *page) { return (__builtin_constant_p((PG_tail)) ? constant_test_bit((PG_tail), (&page->flags)) : variable_test_bit((PG_tail), (&page->flags))); } static inline void __SetPageTail(struct page *page) { __set_bit(PG_tail, &page->flags); } static inline void __ClearPageTail(struct page *page) { __clear_bit(PG_tail, &page->flags); }
static inline int PageSlab(const struct page *page) { return (__builtin_constant_p((PG_slab)) ? constant_test_bit((PG_slab), (&page->flags)) : variable_test_bit((PG_slab), (&page->flags))); } static inline void __SetPageSlab(struct page *page) { __set_bit(PG_slab, &page->flags); } static inline void __ClearPageSlab(struct page *page) { __clear_bit(PG_slab, &page->flags); }





static inline int PageHWPoison(const struct page *page) { return 0; }
# 12 "../include/Xc/mm.h" 2
# 1 "../include/Xc/pfn.h" 1
# 13 "../include/Xc/mm.h" 2
# 21 "../include/Xc/mm.h"
void early_mem_init();
void setup_paging(void);
void free_area_init_nodes(unsigned long *max_zone_pfn);

void get_pfn_range_for_nid(unsigned int nid, unsigned long *start_pfn, unsigned long *end_pfn);

extern unsigned long highest_memmap_pfn;
extern int after_bootmem;
# 62 "../include/Xc/mm.h"
static inline void set_page_zone(struct page *page, enum zone_type zone)
{
    page->flags &= ~(((1UL << 2) - 1) << (((((sizeof(unsigned long) * 8) - 0) - 0) - 2) * (2 != 0)));
 page->flags |= (zone & ((1UL << 2) - 1)) << (((((sizeof(unsigned long) * 8) - 0) - 0) - 2) * (2 != 0));
}

static inline void set_page_node(struct page *page, unsigned long node)
{
    page->flags &= ~(((1UL << 0) - 1) << ((((sizeof(unsigned long) * 8) - 0) - 0) * (0 != 0)));
 page->flags |= (node & ((1UL << 0) - 1)) << ((((sizeof(unsigned long) * 8) - 0) - 0) * (0 != 0));
}

static inline void set_page_links(struct page *page, enum zone_type zone, unsigned long node, unsigned long pfn)
{
    set_page_zone(page, zone);
 set_page_node(page, node);
}

static inline void init_page_count(struct page *page)
{
    atomic_set(&page->_count, 1);
}

static inline void reset_page_mapcount(struct page *page)
{
    atomic_set(&(page)->_mapcount, -1);
}


static inline int page_to_nid (const struct page *page)
{
    return (page->flags >> ((((sizeof(unsigned long) * 8) - 0) - 0) * (0 != 0))) & ((1UL << 0) - 1);
}

static inline enum zone_type page_zonenum(const struct page *page)
{
    return (page->flags >> (((((sizeof(unsigned long) * 8) - 0) - 0) - 2) * (2 != 0))) & ((1UL << 2) - 1);
}

static inline struct zone *page_zone(const struct page *page)
{
    return &(&contig_page_data)->node_zones[page_zonenum(page)];
}

static inline int page_zone_id(struct page *page)
{
    return (page->flags >> ((((((sizeof(unsigned long) * 8) - 0) - 0) < ((((sizeof(unsigned long) * 8) - 0) - 0) - 2)) ? (((sizeof(unsigned long) * 8) - 0) - 0) : ((((sizeof(unsigned long) * 8) - 0) - 0) - 2)) * ((0 + 2) != 0))) & ((1UL << (0 + 2)) - 1);
}





static inline int PageAnon(struct page *page)
{
    return ((unsigned long)page->mapping & 1) != 0;
}

static inline int compound_order(struct page *page)
{
    if (!PageHead(page))
  return 0;
 return (unsigned long)page[1].lru.prev;
}



static inline int PageBuddy(struct page *page)
{
    return atomic_read(&page->_mapcount) == (-128);
}

static inline void __ClearPageBuddy(struct page *page)
{
    atomic_set(&page->_mapcount, -1);
}





static inline unsigned long page_order(struct page *page)
{
    return ((page)->private);
}

static void *lowmem_page_address(const struct page *page)
{
    return ((void*)((u32)(((phys_addr_t)(((unsigned long)((page) - mem_map) + (0UL))) << 12)) + 0xc0000000));
}

static inline struct page *compound_head(struct page *page)
{
    if (PageTail(page))
  return page->first_page;
 return page;
}



static inline struct page *virt_to_head_page(const void *x)
{
    struct page *page = (mem_map + ((((u32)(x) - 0xc0000000) >> 12) - (0UL)));
 return compound_head(page);
}

void put_page(struct page *page);
# 7 "slub.c" 2
# 1 "../include/Xc/percpu.h" 1
# 82 "../include/Xc/percpu.h"
extern void __bad_size_call_parameter(void);
# 8 "slub.c" 2
# 1 "../include/Xc/cpumask.h" 1
# 9 "slub.c" 2
# 1 "../include/Xc/errno.h" 1
# 10 "slub.c" 2

# 1 "../include/Xc/kmemcheck.h" 1




static inline void kmemcheck_alloc_shadow(struct page *page, int order, gfp_t flags, int node) {}
static inline void kmemcheck_free_shadow(struct page *page, int order) {}

static inline void kmemcheck_mark_uninitialized_pages(struct page *p, unsigned int n) {}

static inline void kmemcheck_mark_unallocated_pages(struct page *p, unsigned int n) {}
# 12 "slub.c" 2
# 1 "../include/Xc/vmstat.h" 1






static inline void zone_page_state_add(long x, struct zone *zone, enum zone_stat_item item)
{
    atomic_long_add(x, &zone->vm_stat[item]);

}

static inline void __mod_zone_page_state(struct zone *zone, enum zone_stat_item item, int delta)
{
    zone_page_state_add(delta, zone, item);
}
# 13 "slub.c" 2
# 1 "../include/Xc/poison.h" 1
# 14 "slub.c" 2
# 1 "../include/Xc/current.h" 1
# 15 "slub.c" 2
# 1 "../include/Xc/rcupdate.h" 1



struct rcu_head {
    struct rcu_head *next;
 void (*func)(struct rcu_head *head);
};
# 16 "slub.c" 2

# 1 "../include/Xc/nodemask.h" 1
# 18 "slub.c" 2
# 1 "../include/Xc/swap.h" 1







struct reclaim_state {
    unsigned long reclaimed_slab;
};

static inline init zone_recalim(struct zone *z, gfp_t mask, unsigned int order)
{
    return 0;
}
# 19 "slub.c" 2
# 1 "../include/asm/cache.h" 1
# 20 "slub.c" 2
# 1 "../include/asm/processor.h" 1



# 1 "../include/asm/pagetable.h" 1
# 9 "../include/asm/pagetable.h"
typedef struct {
    u32 pgd;
} pgd_t;

typedef struct {
    u32 pud;
} pud_t;

typedef struct {
    u32 pmd;
} pmd_t;

typedef struct {
    u32 pte;
} pte_t;

typedef struct {
    u32 pgprot;
} pgprot_t;
# 36 "../include/asm/pagetable.h"
extern pgd_t swapper_pg_dir[1024];






static inline pud_t* pud_offset(pgd_t *pgd, u32 address)
{
    return (pud_t*)pgd;
}



static inline pmd_t* pmd_offset(pud_t *pud, u32 address)
{
    return (pmd_t*)pud;
}



static inline pte_t* pte_offset(pmd_t *pmd, u32 address)
{
    return (pte_t*) ((void*)((u32)(pmd->pmd & 0xfffff000) + 0xc0000000)) + (((u32)(address) >> 12) & (1024 - 1));
}

static inline void set_pgd(pgd_t *pgdp, pgd_t pgd)
{
    *pgdp = pgd;
}

static inline void set_pmd(pmd_t *pmdp, pmd_t pmd)
{
    *pmdp = pmd;
}

static inline void set_pte(pte_t *ptep, pte_t pte)
{
    *ptep = pte;
}


static inline pte_t pfn_pte(u32 pfn, u32 pgprot)
{
    return ((pte_t){(pfn << 12 | pgprot)});
}
# 5 "../include/asm/processor.h" 2

static unsigned long __force_order;

static inline unsigned long read_cr3(void)
{
    unsigned long val;
 asm volatile ("mov %%cr3, %0\n\t": "=r"(val), "=m"(__force_order));
 return val;
}

static inline void write_cr3(unsigned long val)
{
    asm volatile ("mov %0, %%cr3": :"r"(val), "m"(__force_order));
}

static inline void __flush_tlb(void)
{
   write_cr3(read_cr3());
}

static inline void __flush_tlb_all(void)
{
    __flush_tlb();
}

static inline void load_cr3(pgd_t *pgdir)
{
    write_cr3(((u32)(pgdir) - 0xc0000000));
}

static inline void rep_nop(void)
{
    asm volatile("rep; nop" ::: "memory");
}

static inline void cpu_relax(void)
{
    rep_nop();
}
# 21 "slub.c" 2
# 1 "../include/asm/cmpxchg_32.h" 1
# 22 "slub.c" 2


static enum {
    DOWN,
 PARTIAL,
 UP,
 SYSFS
} slab_state = DOWN;

static struct list_head slab_caches = {&(slab_caches), &(slab_caches)};
# 50 "slub.c"
static int kmem_size = sizeof(struct kmem_cache);



static int slub_min_order;
static int slub_max_order = 3;
static int slub_min_objects;

struct kmem_cache *kmalloc_caches[(12 + 2)];

static struct kmem_cache *kmem_cache;
static struct kmem_cache *kmalloc_dma_caches[(12 + 2)];

static inline void *get_freepointer(struct kmem_cache *s, void *object)
{
    return *(void **)(object + s->offset);
}

static inline void *get_freepointer_safe(struct kmem_cache *s, void *object)
{
    void *p;
 p = get_freepointer(s, object);
 return p;
}

static inline void set_freepointer(struct kmem_cache *s, void *object, void *fp)
{
    *(void **)(object + s->offset) = fp;
}

static inline int oo_order(struct kmem_cache_order_objects x)
{
    return x.x >> 16;
}

static inline int oo_objects(struct kmem_cache_order_objects x)
{
    return x.x && ((1 << 16) - 1);
}

static inline int order_objects(int order, unsigned long size, int reserved)
{
    return ((((1UL) << 12) << order) - reserved) / size;
}

static inline struct kmem_cache_order_objects oo_make(int order, unsigned long size, int reserved)
{
    struct kmem_cache_order_objects x = {
  (order << 16) + order_objects(order, size, reserved)
 };

 return x;
}




static inline int node_match(struct kmem_cache_cpu *c, int node)
{
    return 1;
}
# 119 "slub.c"
static void slab_out_of_memory(struct kmem_cache *s, gfp_t gfpflags, int nid)
{

}

static inline int alloc_debug_processing(struct kmem_cache *s, struct page *page,
                                   void *object, unsigned long addr)
{
    return 0;
}



static void remove_full(struct kmem_cache *s, struct page *page) {}

static inline void add_full(struct kmem_cache *s, struct kmem_cache_node *n, struct page *page) {}


static inline void add_partial(struct kmem_cache_node*n, struct page *page, int tail)
{
    n->nr_partial++;
 if (tail)
  list_add_tail(&page->lru, &n->partial);
 else
  list_add(&page->lru, &n->partial);
}

static inline void remove_partial(struct kmem_cache_node *n, struct page *page)
{
    list_del(&page->lru);
 n->nr_partial--;
}

static inline void inc_slabs_node(struct kmem_cache *s, int node, int objects) {}
static inline void dec_slabs_node(struct kmem_cache *s, int node, int objects) {}
static inline int slab_pre_alloc_hook(struct kmem_cache *s, gfp_t flags) { return 0; }
static inline void slab_post_alloc_hook(struct kmem_cache *s, gfp_t flags, void *object) {}
static inline void slab_lock(struct page *page)
{
    bit_spin_lock(PG_locked, &page->flags);
}

static inline void slab_unlock(struct page *page)
{
    __bit_spin_unlock(PG_locked, &page->flags);
}



static inline bool __cmpxchg_double_slab(struct kmem_cache *s, struct page *page,
                                   void *freelist_old, unsigned long counters_old,
           void *freelist_new, unsigned long counters_new,
           const char *n)
{
    if (s->flags & 0x40000000UL) {
        if(({ ({ char __ret; __typeof__((counters_old)) __dummy; __typeof__(*((&page->freelist))) __old1 = ((freelist_old)); __typeof__((counters_old)) __old2 = ((counters_old)); __typeof__(*((&page->freelist))) __new1 = ((freelist_new)); __typeof__((counters_old)) __new2 = ((counters_new)); asm volatile("" "cmpxchg8b %2; setz %1" : "=d"(__dummy), "=a"(__ret), "+m"(*(&page->freelist)) : "a"(__old1), "d"(__old2), "b"(__new1), "c"(__new2) : "memory"); __ret; }); })
                                      )
   return 1;
 } else {
        slab_lock(page);
  if (page->freelist == freelist_old && page->counters == counters_old) {
            page->freelist = freelist_new;
   page->counters = counters_new;
   slab_unlock(page);
   return 1;
  }
  slab_unlock(page);
 }

 cpu_relax();


 return 0;
}

static inline bool cmpxchg_double_slab(struct kmem_cache *s, struct page *page,
                                 void *freelist_old, unsigned long counters_old,
            void *freelist_new, unsigned long counters_new, const char *n)
{
    if (s->flags & 0x40000000UL) {
        if (({ ({ char __ret; __typeof__((counters_old)) __dummy; __typeof__(*((&page))) __old1 = ((freelist_old)); __typeof__((counters_old)) __old2 = ((counters_old)); __typeof__(*((&page))) __new1 = ((freelist_new)); __typeof__((counters_old)) __new2 = ((counters__new)); asm volatile("" "cmpxchg8b %2; setz %1" : "=d"(__dummy), "=a"(__ret), "+m"(*(&page)) : "a"(__old1), "d"(__old2), "b"(__new1), "c"(__new2) : "memory"); __ret; }); }))
   return 1;
 } else {
        unsigned long flags;

  do { do { ({ unsigned long __dummy; typeof(flags) __dummy2; (void)(&__dummy == &__dummy2); 1; }); flags = arch_local_irq_save(); } while (0); } while (0);
  slab_lock(page);
  if (page->freelist == freelist_old && page->counters == counters_old) {
            page->freelist = freelist_new;
   page->counters = counters_new;
   slab_unlock(page);
   do { do { ({ unsigned long __dummy; typeof(flags) __dummy2; (void)(&__dummy == &__dummy2); 1; }); arch_local_irq_restore(flags); } while (0); } while (0);
   return 1;
  }

  slab_unlock(page);
  do { do { ({ unsigned long __dummy; typeof(flags) __dummy2; (void)(&__dummy == &__dummy2); 1; }); arch_local_irq_restore(flags); } while (0); } while (0);
 }
 cpu_relax();

 return 0;
}

static inline int acquire_slab(struct kmem_cache *s, struct kmem_cache_node *n,
                         struct page *page)
{
    void *freelist;
 unsigned long counters;
 struct page new;

 do {
        freelist = page->freelist;
  counters = page->counters;
  new.counters = counters;
  new.inuse = page->objects;

  new.frozen = 1;
 } while (!__cmpxchg_double_slab(s, page, freelist, counters, ((void*)0), new.counters,
                        "lock and freeze"));

 remove_partial(n, page);

 if (freelist) {
        do { do { const void *__vpp_verify = (typeof(&((s->cpu_slab->freelist))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof((s->cpu_slab->freelist))) { case 1: do { do { do { (current_thread_info()->preempt_count) += (1); } while (0); __asm__ __volatile__("": : :"memory"); } while (0); *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); }) = ((freelist)); do { do { __asm__ __volatile__("": : :"memory"); do { (current_thread_info()->preempt_count) -= (1); } while (0); } while (0); __asm__ __volatile__("": : :"memory"); do { if (test_ti_thread_flag(current_thread_info(), 3)) preempt_schedule(); } while (0); } while (0); } while (0); break; case 2: do { do { do { (current_thread_info()->preempt_count) += (1); } while (0); __asm__ __volatile__("": : :"memory"); } while (0); *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); }) = ((freelist)); do { do { __asm__ __volatile__("": : :"memory"); do { (current_thread_info()->preempt_count) -= (1); } while (0); } while (0); __asm__ __volatile__("": : :"memory"); do { if (test_ti_thread_flag(current_thread_info(), 3)) preempt_schedule(); } while (0); } while (0); } while (0); break; case 4: do { do { do { (current_thread_info()->preempt_count) += (1); } while (0); __asm__ __volatile__("": : :"memory"); } while (0); *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); }) = ((freelist)); do { do { __asm__ __volatile__("": : :"memory"); do { (current_thread_info()->preempt_count) -= (1); } while (0); } while (0); __asm__ __volatile__("": : :"memory"); do { if (test_ti_thread_flag(current_thread_info(), 3)) preempt_schedule(); } while (0); } while (0); } while (0); break; case 8: do { do { do { (current_thread_info()->preempt_count) += (1); } while (0); __asm__ __volatile__("": : :"memory"); } while (0); *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); }) = ((freelist)); do { do { __asm__ __volatile__("": : :"memory"); do { (current_thread_info()->preempt_count) -= (1); } while (0); } while (0); __asm__ __volatile__("": : :"memory"); do { if (test_ti_thread_flag(current_thread_info(), 3)) preempt_schedule(); } while (0); } while (0); } while (0); break; default: __bad_size_call_parameter(); break; } } while (0);
  do { do { const void *__vpp_verify = (typeof(&((s->cpu_slab->page))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof((s->cpu_slab->page))) { case 1: do { do { do { (current_thread_info()->preempt_count) += (1); } while (0); __asm__ __volatile__("": : :"memory"); } while (0); *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->page)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->page)))))) *)((&(((s->cpu_slab->page))))); }); }) = ((page)); do { do { __asm__ __volatile__("": : :"memory"); do { (current_thread_info()->preempt_count) -= (1); } while (0); } while (0); __asm__ __volatile__("": : :"memory"); do { if (test_ti_thread_flag(current_thread_info(), 3)) preempt_schedule(); } while (0); } while (0); } while (0); break; case 2: do { do { do { (current_thread_info()->preempt_count) += (1); } while (0); __asm__ __volatile__("": : :"memory"); } while (0); *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->page)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->page)))))) *)((&(((s->cpu_slab->page))))); }); }) = ((page)); do { do { __asm__ __volatile__("": : :"memory"); do { (current_thread_info()->preempt_count) -= (1); } while (0); } while (0); __asm__ __volatile__("": : :"memory"); do { if (test_ti_thread_flag(current_thread_info(), 3)) preempt_schedule(); } while (0); } while (0); } while (0); break; case 4: do { do { do { (current_thread_info()->preempt_count) += (1); } while (0); __asm__ __volatile__("": : :"memory"); } while (0); *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->page)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->page)))))) *)((&(((s->cpu_slab->page))))); }); }) = ((page)); do { do { __asm__ __volatile__("": : :"memory"); do { (current_thread_info()->preempt_count) -= (1); } while (0); } while (0); __asm__ __volatile__("": : :"memory"); do { if (test_ti_thread_flag(current_thread_info(), 3)) preempt_schedule(); } while (0); } while (0); } while (0); break; case 8: do { do { do { (current_thread_info()->preempt_count) += (1); } while (0); __asm__ __volatile__("": : :"memory"); } while (0); *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->page)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->page)))))) *)((&(((s->cpu_slab->page))))); }); }) = ((page)); do { do { __asm__ __volatile__("": : :"memory"); do { (current_thread_info()->preempt_count) -= (1); } while (0); } while (0); __asm__ __volatile__("": : :"memory"); do { if (test_ti_thread_flag(current_thread_info(), 3)) preempt_schedule(); } while (0); } while (0); } while (0); break; default: __bad_size_call_parameter(); break; } } while (0);
  do { do { const void *__vpp_verify = (typeof(&((s->cpu_slab->node))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof((s->cpu_slab->node))) { case 1: do { do { do { (current_thread_info()->preempt_count) += (1); } while (0); __asm__ __volatile__("": : :"memory"); } while (0); *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->node)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->node)))))) *)((&(((s->cpu_slab->node))))); }); }) = ((page_to_nid(page))); do { do { __asm__ __volatile__("": : :"memory"); do { (current_thread_info()->preempt_count) -= (1); } while (0); } while (0); __asm__ __volatile__("": : :"memory"); do { if (test_ti_thread_flag(current_thread_info(), 3)) preempt_schedule(); } while (0); } while (0); } while (0); break; case 2: do { do { do { (current_thread_info()->preempt_count) += (1); } while (0); __asm__ __volatile__("": : :"memory"); } while (0); *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->node)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->node)))))) *)((&(((s->cpu_slab->node))))); }); }) = ((page_to_nid(page))); do { do { __asm__ __volatile__("": : :"memory"); do { (current_thread_info()->preempt_count) -= (1); } while (0); } while (0); __asm__ __volatile__("": : :"memory"); do { if (test_ti_thread_flag(current_thread_info(), 3)) preempt_schedule(); } while (0); } while (0); } while (0); break; case 4: do { do { do { (current_thread_info()->preempt_count) += (1); } while (0); __asm__ __volatile__("": : :"memory"); } while (0); *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->node)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->node)))))) *)((&(((s->cpu_slab->node))))); }); }) = ((page_to_nid(page))); do { do { __asm__ __volatile__("": : :"memory"); do { (current_thread_info()->preempt_count) -= (1); } while (0); } while (0); __asm__ __volatile__("": : :"memory"); do { if (test_ti_thread_flag(current_thread_info(), 3)) preempt_schedule(); } while (0); } while (0); } while (0); break; case 8: do { do { do { (current_thread_info()->preempt_count) += (1); } while (0); __asm__ __volatile__("": : :"memory"); } while (0); *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->node)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->node)))))) *)((&(((s->cpu_slab->node))))); }); }) = ((page_to_nid(page))); do { do { __asm__ __volatile__("": : :"memory"); do { (current_thread_info()->preempt_count) -= (1); } while (0); } while (0); __asm__ __volatile__("": : :"memory"); do { if (test_ti_thread_flag(current_thread_info(), 3)) preempt_schedule(); } while (0); } while (0); } while (0); break; default: __bad_size_call_parameter(); break; } } while (0);
  return 1;
 } else {
        return 0;
 }
}

static struct page *get_partial_node(struct kmem_cache *s, struct kmem_cache_node *n)
{
    struct page *page;

 if (!n || !n->nr_partial)
  return ((void*)0);

 spin_lock(&n->list_lock);
    for (page = ({ const typeof(((typeof(*page) *)0)->lru) * __mptr = ((&n->partial)->next); (typeof(*page) *)((char *)__mptr - ((size_t) &((typeof(*page) *)0)->lru));}); &page->lru != (&n->partial); page = ({ const typeof(((typeof(*page) *)0)->lru) * __mptr = (page->lru.next); (typeof(*page) *)((char *)__mptr - ((size_t) &((typeof(*page) *)0)->lru));}))
  if (acquire_slab(s, n, page))
   goto out;

 page = ((void*)0);
out:
 spin_unlock(&n->list_lock);
 return page;
}

static inline struct kmem_cache_node *get_node(struct kmem_cache *s, int node)
{
    return s->node[node];
}

static inline struct page *get_any_partial(struct kmem_cache *s, gfp_t flags)
{
    return ((void*)0);
}

static struct page *get_partial(struct kmem_cache *s, gfp_t flags, int node)
{
    struct page *page;
 int searchnode = (node == (-1)) ? numa_node_id() : node;

 page = get_partial_node(s, get_node(s, searchnode));
 if (page || node != (-1))
  return page;

 return get_any_partial(s, flags);
}


int slab_is_available(void)
{
    return slab_state >= UP;
}



static inline unsigned long next_tid(unsigned long tid)
{
    return tid + ( __builtin_constant_p(1) ? ( (1 == 1) ? 1 : (1UL << (( __builtin_constant_p((1) - 1) ? ( ((1) - 1) < 1 ? ____ilog2_NaN() : ((1) - 1) & (1ULL << 63) ? 63 : ((1) - 1) & (1ULL << 62) ? 62 : ((1) - 1) & (1ULL << 61) ? 61 : ((1) - 1) & (1ULL << 60) ? 60 : ((1) - 1) & (1ULL << 59) ? 59 : ((1) - 1) & (1ULL << 58) ? 58 : ((1) - 1) & (1ULL << 57) ? 57 : ((1) - 1) & (1ULL << 56) ? 56 : ((1) - 1) & (1ULL << 55) ? 55 : ((1) - 1) & (1ULL << 54) ? 54 : ((1) - 1) & (1ULL << 53) ? 53 : ((1) - 1) & (1ULL << 52) ? 52 : ((1) - 1) & (1ULL << 51) ? 51 : ((1) - 1) & (1ULL << 50) ? 50 : ((1) - 1) & (1ULL << 49) ? 49 : ((1) - 1) & (1ULL << 48) ? 48 : ((1) - 1) & (1ULL << 47) ? 47 : ((1) - 1) & (1ULL << 46) ? 46 : ((1) - 1) & (1ULL << 45) ? 45 : ((1) - 1) & (1ULL << 44) ? 44 : ((1) - 1) & (1ULL << 43) ? 43 : ((1) - 1) & (1ULL << 42) ? 42 : ((1) - 1) & (1ULL << 41) ? 41 : ((1) - 1) & (1ULL << 40) ? 40 : ((1) - 1) & (1ULL << 39) ? 39 : ((1) - 1) & (1ULL << 38) ? 38 : ((1) - 1) & (1ULL << 37) ? 37 : ((1) - 1) & (1ULL << 36) ? 36 : ((1) - 1) & (1ULL << 35) ? 35 : ((1) - 1) & (1ULL << 34) ? 34 : ((1) - 1) & (1ULL << 33) ? 33 : ((1) - 1) & (1ULL << 32) ? 32 : ((1) - 1) & (1ULL << 31) ? 31 : ((1) - 1) & (1ULL << 30) ? 30 : ((1) - 1) & (1ULL << 29) ? 29 : ((1) - 1) & (1ULL << 28) ? 28 : ((1) - 1) & (1ULL << 27) ? 27 : ((1) - 1) & (1ULL << 26) ? 26 : ((1) - 1) & (1ULL << 25) ? 25 : ((1) - 1) & (1ULL << 24) ? 24 : ((1) - 1) & (1ULL << 23) ? 23 : ((1) - 1) & (1ULL << 22) ? 22 : ((1) - 1) & (1ULL << 21) ? 21 : ((1) - 1) & (1ULL << 20) ? 20 : ((1) - 1) & (1ULL << 19) ? 19 : ((1) - 1) & (1ULL << 18) ? 18 : ((1) - 1) & (1ULL << 17) ? 17 : ((1) - 1) & (1ULL << 16) ? 16 : ((1) - 1) & (1ULL << 15) ? 15 : ((1) - 1) & (1ULL << 14) ? 14 : ((1) - 1) & (1ULL << 13) ? 13 : ((1) - 1) & (1ULL << 12) ? 12 : ((1) - 1) & (1ULL << 11) ? 11 : ((1) - 1) & (1ULL << 10) ? 10 : ((1) - 1) & (1ULL << 9) ? 9 : ((1) - 1) & (1ULL << 8) ? 8 : ((1) - 1) & (1ULL << 7) ? 7 : ((1) - 1) & (1ULL << 6) ? 6 : ((1) - 1) & (1ULL << 5) ? 5 : ((1) - 1) & (1ULL << 4) ? 4 : ((1) - 1) & (1ULL << 3) ? 3 : ((1) - 1) & (1ULL << 2) ? 2 : ((1) - 1) & (1ULL << 1) ? 1 : ((1) - 1) & (1ULL << 0) ? 0 : ____ilog2_NaN() ) : (sizeof((1) - 1) <= 4) ? __ilog2_u32((1) - 1) : __ilog2_u64((1) - 1) ) + 1)) ) : __roundup_pow_of_two(1) );
}

static inline unsigned long kmem_cache_flags(unsigned long objsize, unsigned long flags,
                                       const char *name, void (*ctor)(void *))
{
    return flags;
}

static unsigned long calculate_alignment(unsigned long flags, unsigned long align,
                                   unsigned long size)
{
    if (flags & 0x00002000UL) {
        unsigned long ralign = (1 << 5);
  while (size <= (ralign >> 1))
   ralign >>= 1;
  align = ({ typeof(align) _max1 = (align); typeof(ralign) _max2 = (ralign); (void) (&_max1 == &_max2); _max1 > _max2 ? _max1 : _max2;});
 }

 if (align < __alignof__(unsigned long long))
  align = __alignof__(unsigned long long);

 return (((align) + ((typeof(align))(sizeof(void *)) - 1)) & ~((typeof(align))(sizeof(void *)) - 1));
}


static inline int slab_order(int size, int min_objects, int max_order,
                       int fract_leftover, int reserved)
{
    int order;
 int rem;
 int min_order = slub_min_order;

 if (order_objects(min_order, size, reserved) > 0x7FFF)
  return get_order(size * 0x7FFF) - 1;

 for (order = ({ typeof(min_order) _max1 = (min_order); typeof(fls(min_objects * size - 1) - 12) _max2 = (fls(min_objects * size - 1) - 12); (void) (&_max1 == &_max2); _max1 > _max2 ? _max1 : _max2;});
   order <= max_order; order++) {
        unsigned long slab_size = ((1UL) << 12) << order;
  if (slab_size < min_objects * size + reserved)
   continue;

  rem = (slab_size - reserved) % size;
  if (rem <= slab_size / fract_leftover)
   break;
 }

 return order;
}

static inline int calculate_order(int size, int reserved)
{
    int order;
 int min_objects;
 int fraction;
 int max_objects;

 min_objects = slub_min_objects;
 if (!min_objects)
  min_objects = 4 * (fls(1) + 1);
 max_objects = order_objects(slub_max_order, size, reserved);
 min_objects = ({ typeof(min_objects) _min1 = (min_objects); typeof(max_objects) _min2 = (max_objects); (void) (&_min1 == &_min2); _min1 > _min2 ? _min1 : _min2;});

 while (min_objects > 1) {
        fraction = 16;
  while (fraction >= 4) {
            order = slab_order(size, min_objects, slub_max_order, fraction, reserved);
   if (order <= slub_max_order)
    return order;
   fraction >>= 1;
  }
  min_objects--;
 }

 order = slab_order(size, 1, slub_max_order, 1, reserved);
 if (order <= slub_max_order)
  return order;

 order = slab_order(size, 1, 11, 1, reserved);
 if (order < 11)
  return order;
 return -38;
}


static int calculate_sizes(struct kmem_cache *s, int forced_order)
{
    unsigned long flags = s->flags;
 unsigned long size = s->objsize;
 unsigned long align = s->align;
 int order;

 size = (((size) + ((typeof(size))(sizeof(void *)) - 1)) & ~((typeof(size))(sizeof(void *)) - 1));
 s->inuse = size;

 if (((flags & (0x00080000UL | 0x00000800UL)) || s->ctor)) {
        s->offset = size;
  size += sizeof(void *);
 }

 align = calculate_alignment(flags, align, s->objsize);
 s->align = align;

 size = (((size) + ((typeof(size))(align) - 1)) & ~((typeof(size))(align) - 1));
 s->size = size;

 if (forced_order >= 0)
  order = forced_order;
 else
  order = calculate_order(size, s->reserved);

 if (order < 0)
  return 0;

 s->allocflags = 0;
 if (order)
  s->allocflags |= ((gfp_t)0x4000u);

 if (s->flags & 0x00004000UL)
  s->allocflags |= ((gfp_t)0x01u);

 if (s->flags & 0x00020000UL)
  s->allocflags |= ((gfp_t)0x80000u);

 s->oo = oo_make(order, size, s->reserved);
 s->min = oo_make(get_order(size), size, s->reserved);
    if(oo_objects(s->oo) > oo_objects(s->max))
  s->max = s->oo;

    return !!oo_objects(s->oo);
}

static void set_min_partial(struct kmem_cache *s ,unsigned long min)
{
    if (min < 5)
  min = 5;
  else if (min > 10)
  min = 10;
 s->min_partial = min;
}

static inline struct page *alloc_slab_page(gfp_t flags, int node, struct kmem_cache_order_objects oo)
{
    int order = oo_order(oo);
 flags |= ((gfp_t)0);
 if (node == (-1))
  return alloc_pages_node(numa_node_id(), flags, order);
 else
  return alloc_pages_exact_node(node, flags, order);
}

static struct page *allocate_slab(struct kmem_cache *s, gfp_t flags, int node)
{
    struct page *page;
 struct kmem_cache_order_objects oo = s->oo;
 gfp_t alloc_gfp;

 flags &= gfp_allowed_mask;

 if (flags & ((gfp_t)0x10u))
  do { arch_local_irq_enable(); } while (0);

 flags |= s->allocflags;

    alloc_gfp = (flags | ((gfp_t)0x200u) | ((gfp_t)0x1000u)) & ~((gfp_t)0x800u);

 page = alloc_slab_page(alloc_gfp, node, oo);
 if (!page) {
        oo = s->min;
  page = alloc_slab_page(flags, node, oo);





 }

 if (flags & ((gfp_t)0x10u))
  do { arch_local_irq_disable(); } while (0);
 if (!page)
  return ((void*)0);

 if (0 && !(s->flags & (0x00000000UL | (0x00000100UL | 0x00000400UL | 0x00000800UL | 0x00010000UL)))) {
        int pages = 1 << oo_order(oo);
  kmemcheck_alloc_shadow(page, oo_order(oo), flags, node);

  if (s->ctor)
   kmemcheck_mark_uninitialized_pages(page, pages);
  else
   kmemcheck_mark_unallocated_pages(page, pages);
 }

 page->objects = oo_objects(oo);
 __mod_zone_page_state(page_zone(page),
               (s->flags & 0x00020000UL) ?
       NR_SLAB_RECLAIMABLE : NR_SLAB_UNRECLAIMABLE, 1 << oo_order(oo));
 return page;
}


static void setup_object(struct kmem_cache *s, struct page *page, void *object)
{

 if (s->ctor)
  s->ctor(object);
}

static struct page *new_slab(struct kmem_cache *s, gfp_t flags, int node)
{
    struct page *page;
 void *start;
 void *last;
 void *p;

 page = allocate_slab(s, flags & ((((gfp_t)0x10u) | ((gfp_t)0x20u) | ((gfp_t)0x40u) | ((gfp_t)0x80u) | ((gfp_t)0x200u) | ((gfp_t)0x400u) | ((gfp_t)0x800u) | ((gfp_t)0x1000u) | ((gfp_t)0x10000u)) | (((gfp_t)0x20000u) | ((gfp_t)0x40000u))), node);
    if (!page)
  goto out;

 inc_slabs_node(s, page_to_nid(page), page->objects);
 page->slab = s;
 page->flags |= 1 << PG_slab;

 start = lowmem_page_address(page);

 if (s->flags & 0x00000800UL)
  memset(start, 0x5a, ((1UL) << 12) << compound_order(page));

 last = start;
 for (p = (start); p < (start) + (page->objects) * (s)->size; p += (s)->size) {
        setup_object(s, page, last);
  set_freepointer(s, last, p);
  last = p;
 }

 setup_object(s, page, last);
 set_freepointer(s, last, ((void*)0));

 page->freelist = start;
 page->inuse = 0;
 page->frozen = 1;
out:
 return page;
}

static void init_kmem_cache_node(struct kmem_cache_node *n, struct kmem_cache *s)
{
    n->nr_partial = 0;
 do { spinlock_check(&n->list_lock); do { *(&(&n->list_lock)->rlock) = (raw_spinlock_t) { .raw_lock = {1}, .magic = 0xdead4ead, .owner_cpu = -1, .owner = ((void*)-1L), }; } while (0); } while (0);
 INIT_LIST_HEAD(&n->partial);






}



static void __free_slab(struct kmem_cache *s, struct page *page)
{
    int order = compound_order(page);
 int pages = 1 << order;
# 576 "slub.c"
 __ClearPageSlab(page);
 reset_page_mapcount(page);
 if ((current_thread_info()->task))

 __free_pages(page, order);
}

static void free_slab(struct kmem_cache *s, struct page *page)
{
    if (s->flags & 0x00080000UL) {
# 599 "slub.c"
 } else
  __free_slab(s, page);
}

static void discard_slab(struct kmem_cache *s, struct page *page)
{
    dec_slabs_node(s, page_to_nid(page), page->objects);
 free_slab(s, page);
}

static void deactivate_slab(struct kmem_cache *s, struct kmem_cache_cpu *c)
{
    enum slab_modes {M_NONE, M_PARTIAL, M_FULL, M_FREE};
    struct page *page = c->page;
 struct kmem_cache_node *n = get_node(s, page_to_nid(page));
 int lock = 0;

 enum slab_modes l = M_NONE, m = M_NONE;
 void *freelist;
 void *nextfree;
 int tail = 0;
 struct page new;
 struct page old;

 if (page->freelist) {

  tail = 1;
 }

    c->tid = next_tid(c->tid);
 c->page = ((void*)0);
 freelist = c->freelist;
 c->freelist = ((void*)0);

 while (freelist && (nextfree = get_freepointer(s, freelist))) {
        void *prior;
  unsigned long counters;

  do {
   prior = page->freelist;
   counters = page->counters;
   set_freepointer(s, freelist, prior);
   new.counters = counters;
   new.inuse--;
  } while (!__cmpxchg_double_slab(s, page, prior, counters, freelist, new.counters,
     "drain percpu freelist"));
  freelist = nextfree;
 }

redo:
 old.freelist = page->freelist;
 old.counters = page->counters;
 new.counters = old.counters;
 if (freelist) {
        new.inuse--;
  set_freepointer(s, freelist, old.freelist);
  new.freelist = freelist;
 } else
  new.freelist = old.freelist;

 new.frozen = 0;

 if (!new.inuse && n->nr_partial > s->min_partial)
  m = M_FREE;
 else if (new.freelist) {
        m = M_PARTIAL;
  if (!lock) {
            lock = 1;
   spin_lock(&n->list_lock);
  }
 } else {
        m = M_FULL;






 }

 if (l != m) {
        if (l == M_PARTIAL)
   remove_partial(n, page);
  else if (l == M_FULL)
   remove_full(s, page);

  if (m == M_PARTIAL) {
            add_partial(n, page, tail);

  } else if (m == M_FULL) {

   add_full(s, n, page);
  }
 }

 l = m;

 if (!__cmpxchg_double_slab(s, page, old.freelist, old.counters, new.freelist, new.counters,
    "unfreezing slab"))
  goto redo;

 if (lock)
  spin_lock(&n->list_lock);
 if (m == M_FREE) {

  discard_slab(s, page);

 }
}

static inline void flush_slab(struct kmem_cache *s, struct kmem_cache_cpu *c)
{

 deactivate_slab(s, c);
}


static void *__slab_alloc(struct kmem_cache *s, gfp_t gfpflags, int node,
                    unsigned long addr, struct kmem_cache_cpu *c)
{
    void **object;
 struct page *page;
 unsigned long flags;
 struct page new;
 unsigned long counters;

 do { do { ({ unsigned long __dummy; typeof(flags) __dummy2; (void)(&__dummy == &__dummy2); 1; }); flags = arch_local_irq_save(); } while (0); } while (0);

 c = ({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((s->cpu_slab))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((s->cpu_slab))) *)((s->cpu_slab)); }); });

 gfpflags &= ~((gfp_t)0x8000u);
 page = c->page;
 if (!page)
  goto new_slab;

 if (!node_match(c, node)) {

  deactivate_slab(s, c);
  goto new_slab;
 }


 do {
        object = page->freelist;
  counters = page->counters;
  new.counters = counters;

  new.inuse = page->objects;
  new.frozen = object != ((void*)0);
 } while (!__cmpxchg_double_slab(s, page, object, counters, ((void*)0), new.counters,
                        "__slab_alloc"));

 if (!object) {
        c->page = ((void*)0);

  goto new_slab;
 }



load_freelist:
 c->freelist = get_freepointer(s, object);
 c->tid = next_tid(c->tid);
 do { do { ({ unsigned long __dummy; typeof(flags) __dummy2; (void)(&__dummy == &__dummy2); 1; }); arch_local_irq_restore(flags); } while (0); } while (0);
 return object;

new_slab:
 page = get_partial(s, gfpflags, node);
 if (page) {

  object = c->freelist;





  goto load_freelist;
 }

 page = new_slab(s, gfpflags, node);
 if (page) {
        c = ({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((s->cpu_slab))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((s->cpu_slab))) *)((s->cpu_slab)); }); });
  if (c->page)
   flush_slab(s, c);

  object = page->freelist;
  page->freelist = ((void*)0);
  page->inuse = page->objects;


  c->node = page_to_nid(page);
  c->page = page;





  goto load_freelist;
 }

 if (!(gfpflags & ((gfp_t)0x200u)) && printk_ratelimit())
  slab_out_of_memory(s, gfpflags, node);
 do { do { ({ unsigned long __dummy; typeof(flags) __dummy2; (void)(&__dummy == &__dummy2); 1; }); arch_local_irq_restore(flags); } while (0); } while (0);
 return ((void*)0);

debug:
 if (!object || !alloc_debug_processing(s, page, object, addr))
  goto new_slab;

 c->freelist = get_freepointer(s, object);
 deactivate_slab(s, c);
 c->page = ((void*)0);
 c->node = (-1);
 do { do { ({ unsigned long __dummy; typeof(flags) __dummy2; (void)(&__dummy == &__dummy2); 1; }); arch_local_irq_restore(flags); } while (0); } while (0);
 return object;
}

static inline void *slab_alloc(struct kmem_cache *s, gfp_t gfpflags, int node,
                         unsigned long addr)
{
    void **object;
 struct kmem_cache_cpu *c;
 unsigned long tid;

 if (slab_pre_alloc_hook(s, gfpflags))
  return ((void*)0);

redo:
 c = ({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((s->cpu_slab))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((s->cpu_slab))) *)((s->cpu_slab)); }); });
    tid = c->tid;
 __asm__ __volatile__("": : :"memory");

 object = c->freelist;
 if (!object || !node_match(c, node))
  object = __slab_alloc(s, gfpflags, node, addr, c);
 else {
        if (!({ bool pdcrb_ret__; do { const void *__vpp_verify = (typeof(&(s->cpu_slab->freelist)))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof((s->cpu_slab->freelist))) { case 1: pdcrb_ret__ =

 ({ int ret__; unsigned long flags; do { do { ({ unsigned long __dummy; typeof(flags) __dummy2; (void)(&__dummy == &__dummy2); 1; }); flags = arch_local_irq_save(); } while (0); } while (0); ret__ = ({ int __ret = 0; if (({ typeof(((s->cpu_slab->freelist))) pscr_ret__; do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->freelist)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->freelist)))) { case 1: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; case 2: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; case 4: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; case 8: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; default: __bad_size_call_parameter(); break; } pscr_ret__; }) == ((object)) && ({ typeof(((s->cpu_slab->tid))) pscr_ret__; do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->tid)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->tid)))) { case 1: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; case 2: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; case 4: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; case 8: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; default: __bad_size_call_parameter(); break; } pscr_ret__; }) == ((tid))) { do { do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->freelist)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->freelist)))) { case 1: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((get_freepointer_safe(s, object))))); } while (0); break; case 2: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((get_freepointer_safe(s, object))))); } while (0); break; case 4: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((get_freepointer_safe(s, object))))); } while (0); break; case 8: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((get_freepointer_safe(s, object))))); } while (0); break; default: __bad_size_call_parameter(); break; } } while (0); do { do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->tid)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->tid)))) { case 1: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; case 2: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; case 4: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; case 8: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; default: __bad_size_call_parameter(); break; } } while (0); __ret = 1; } (__ret); }); do { do { ({ unsigned long __dummy; typeof(flags) __dummy2; (void)(&__dummy == &__dummy2); 1; }); arch_local_irq_restore(flags); } while (0); } while (0); ret__; }); break; case 2: pdcrb_ret__ = ({ int ret__; unsigned long flags; do { do { ({ unsigned long __dummy; typeof(flags) __dummy2; (void)(&__dummy == &__dummy2); 1; }); flags = arch_local_irq_save(); } while (0); } while (0); ret__ = ({ int __ret = 0; if (({ typeof(((s->cpu_slab->freelist))) pscr_ret__; do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->freelist)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->freelist)))) { case 1: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; case 2: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; case 4: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; case 8: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; default: __bad_size_call_parameter(); break; } pscr_ret__; }) == ((object)) && ({ typeof(((s->cpu_slab->tid))) pscr_ret__; do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->tid)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->tid)))) { case 1: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; case 2: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; case 4: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; case 8: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; default: __bad_size_call_parameter(); break; } pscr_ret__; }) == ((tid))) { do { do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->freelist)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->freelist)))) { case 1: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((get_freepointer_safe(s, object))))); } while (0); break; case 2: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((get_freepointer_safe(s, object))))); } while (0); break; case 4: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((get_freepointer_safe(s, object))))); } while (0); break; case 8: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((get_freepointer_safe(s, object))))); } while (0); break; default: __bad_size_call_parameter(); break; } } while (0); do { do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->tid)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->tid)))) { case 1: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; case 2: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; case 4: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; case 8: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; default: __bad_size_call_parameter(); break; } } while (0); __ret = 1; } (__ret); }); do { do { ({ unsigned long __dummy; typeof(flags) __dummy2; (void)(&__dummy == &__dummy2); 1; }); arch_local_irq_restore(flags); } while (0); } while (0); ret__; }); break; case 4: pdcrb_ret__ = ({ int ret__; unsigned long flags; do { do { ({ unsigned long __dummy; typeof(flags) __dummy2; (void)(&__dummy == &__dummy2); 1; }); flags = arch_local_irq_save(); } while (0); } while (0); ret__ = ({ int __ret = 0; if (({ typeof(((s->cpu_slab->freelist))) pscr_ret__; do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->freelist)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->freelist)))) { case 1: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; case 2: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; case 4: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; case 8: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; default: __bad_size_call_parameter(); break; } pscr_ret__; }) == ((object)) && ({ typeof(((s->cpu_slab->tid))) pscr_ret__; do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->tid)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->tid)))) { case 1: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; case 2: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; case 4: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; case 8: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; default: __bad_size_call_parameter(); break; } pscr_ret__; }) == ((tid))) { do { do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->freelist)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->freelist)))) { case 1: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((get_freepointer_safe(s, object))))); } while (0); break; case 2: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((get_freepointer_safe(s, object))))); } while (0); break; case 4: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((get_freepointer_safe(s, object))))); } while (0); break; case 8: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((get_freepointer_safe(s, object))))); } while (0); break; default: __bad_size_call_parameter(); break; } } while (0); do { do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->tid)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->tid)))) { case 1: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; case 2: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; case 4: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; case 8: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; default: __bad_size_call_parameter(); break; } } while (0); __ret = 1; } (__ret); }); do { do { ({ unsigned long __dummy; typeof(flags) __dummy2; (void)(&__dummy == &__dummy2); 1; }); arch_local_irq_restore(flags); } while (0); } while (0); ret__; }); break; case 8: pdcrb_ret__ = ({ int ret__; unsigned long flags; do { do { ({ unsigned long __dummy; typeof(flags) __dummy2; (void)(&__dummy == &__dummy2); 1; }); flags = arch_local_irq_save(); } while (0); } while (0); ret__ = ({ int __ret = 0; if (({ typeof(((s->cpu_slab->freelist))) pscr_ret__; do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->freelist)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->freelist)))) { case 1: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; case 2: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; case 4: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; case 8: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; default: __bad_size_call_parameter(); break; } pscr_ret__; }) == ((object)) && ({ typeof(((s->cpu_slab->tid))) pscr_ret__; do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->tid)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->tid)))) { case 1: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; case 2: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; case 4: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; case 8: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; default: __bad_size_call_parameter(); break; } pscr_ret__; }) == ((tid))) { do { do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->freelist)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->freelist)))) { case 1: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((get_freepointer_safe(s, object))))); } while (0); break; case 2: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((get_freepointer_safe(s, object))))); } while (0); break; case 4: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((get_freepointer_safe(s, object))))); } while (0); break; case 8: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((get_freepointer_safe(s, object))))); } while (0); break; default: __bad_size_call_parameter(); break; } } while (0); do { do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->tid)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->tid)))) { case 1: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; case 2: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; case 4: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; case 8: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; default: __bad_size_call_parameter(); break; } } while (0); __ret = 1; } (__ret); }); do { do { ({ unsigned long __dummy; typeof(flags) __dummy2; (void)(&__dummy == &__dummy2); 1; }); arch_local_irq_restore(flags); } while (0); } while (0); ret__; }); break; default: __bad_size_call_parameter(); break; } pdcrb_ret__; })) {

   goto redo;
  }

 }
 if ((gfpflags & ((gfp_t)0x8000u)) && object)
  memset(object, 0, s->objsize);

 slab_post_alloc_hook(s, gfpflags, object);

 return object;
}

static u8 size_index[24] = {
    3,
 4,
 5,
 5,
 6,
 6,
 6,
 6,
 1,
 1,
 1,
 1,
 7,
 7,
 7,
 7,
 2,
 2,
 2,
 2,
 2,
 2,
 2,
 2,
};

static inline int size_index_elem(size_t bytes)
{
    return (bytes - 1) / 8;
}

static struct kmem_cache *get_slab(size_t size, gfp_t flags)
{
    int index;

 if (size <= 192) {
        if (!size)
   return ((void *)16);
  index = size_index[size_index_elem(size)];
 } else
        index = fls(size - 1);

 if (flags & ((gfp_t)0x01u))
  return kmalloc_dma_caches[index];

 return kmalloc_caches[index];
}

void *__kmalloc(size_t size, gfp_t flags)
{
    struct kmem_cache *s;
 void *ret;

 if (size > (2 * ((1UL) << 12)))
  return kmalloc_large(size, flags);

 s = get_slab(size, flags);
 if (((unsigned long)(s) <= (unsigned long)((void *)16)))
  return s;

 ret = slab_alloc(s, flags, (-1), (unsigned long)__builtin_return_address(0));

 return ret;
}

void *kmem_cache_alloc(struct kmem_cache *s, gfp_t gfpflags)
{
    void *ret = slab_alloc(s, gfpflags, (-1), (unsigned long)__builtin_return_address(0));


 return ret;
}


static void __slab_free(struct kmem_cache *s, struct page *page, void *x,
                  unsigned long addr)
{
    void *prior;
 void **object = (void *)x;
 int was_frozen;
 int inuse;
 struct page new;
 unsigned long counters;
 struct kmem_cache_node *n = ((void*)0);
 unsigned long flags = flags;





 do {
        prior = page->freelist;
  counters = page->counters;
  set_freepointer(s, object, prior);
  new.counters = counters;
  was_frozen = new.frozen;
  new.inuse--;
  if ((!new.inuse || !prior) && !was_frozen && !n) {
            n = get_node(s, page_to_nid(page));

   do { do { ({ unsigned long __dummy; typeof(flags) __dummy2; (void)(&__dummy == &__dummy2); 1; }); do { do { do { ({ unsigned long __dummy; typeof(flags) __dummy2; (void)(&__dummy == &__dummy2); 1; }); flags = arch_local_irq_save(); } while (0); } while (0); do { do { do { (current_thread_info()->preempt_count) += (1); } while (0); __asm__ __volatile__("": : :"memory"); } while (0); (void)(spinlock_check(&n->list_lock)); } while (0); } while (0); } while (0); } while (0);
  }
  inuse = new.inuse;
 } while (!cmpxchg_double_slab(s, page, prior, counters, object, new.counters,
                      "__slab_free"));

 if (!n) {




  return;
 }

 if (was_frozen) {

 }
 else {
        if (!inuse && n->nr_partial > s->min_partial)
   goto slab_empty;

  if (!prior) {
            remove_full(s, page);
   add_partial(n, page, 1);

  }
 }

 spin_unlock_irqrestore(&n->list_lock, flags);
 return;

slab_empty:
 if (prior) {
        remove_partial(n, page);

 } else
  remove_full(s, page);

 spin_unlock_irqrestore(&n->list_lock, flags);

 discard_slab(s, page);
}

static void slab_free(struct kmem_cache *s, struct page *page, void *x,
                unsigned long addr)
{
    void **object = (void *)x;
 struct kmem_cache_cpu *c;
 unsigned long tid;


redo:
 c = ({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((s->cpu_slab))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((s->cpu_slab))) *)((s->cpu_slab)); }); });
 tid = c->tid;
 __asm__ __volatile__("": : :"memory");

 if (page == c->page) {
        set_freepointer(s, object, c->freelist);
  if (!({ bool pdcrb_ret__; do { const void *__vpp_verify = (typeof(&(s->cpu_slab->freelist)))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof((s->cpu_slab->freelist))) { case 1: pdcrb_ret__ =
 ({ int ret__; unsigned long flags; do { do { ({ unsigned long __dummy; typeof(flags) __dummy2; (void)(&__dummy == &__dummy2); 1; }); flags = arch_local_irq_save(); } while (0); } while (0); ret__ = ({ int __ret = 0; if (({ typeof(((s->cpu_slab->freelist))) pscr_ret__; do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->freelist)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->freelist)))) { case 1: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; case 2: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; case 4: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; case 8: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; default: __bad_size_call_parameter(); break; } pscr_ret__; }) == ((c->freelist)) && ({ typeof(((s->cpu_slab->tid))) pscr_ret__; do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->tid)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->tid)))) { case 1: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; case 2: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; case 4: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; case 8: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; default: __bad_size_call_parameter(); break; } pscr_ret__; }) == ((tid))) { do { do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->freelist)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->freelist)))) { case 1: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((object)))); } while (0); break; case 2: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((object)))); } while (0); break; case 4: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((object)))); } while (0); break; case 8: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((object)))); } while (0); break; default: __bad_size_call_parameter(); break; } } while (0); do { do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->tid)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->tid)))) { case 1: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; case 2: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; case 4: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; case 8: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; default: __bad_size_call_parameter(); break; } } while (0); __ret = 1; } (__ret); }); do { do { ({ unsigned long __dummy; typeof(flags) __dummy2; (void)(&__dummy == &__dummy2); 1; }); arch_local_irq_restore(flags); } while (0); } while (0); ret__; }); break; case 2: pdcrb_ret__ = ({ int ret__; unsigned long flags; do { do { ({ unsigned long __dummy; typeof(flags) __dummy2; (void)(&__dummy == &__dummy2); 1; }); flags = arch_local_irq_save(); } while (0); } while (0); ret__ = ({ int __ret = 0; if (({ typeof(((s->cpu_slab->freelist))) pscr_ret__; do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->freelist)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->freelist)))) { case 1: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; case 2: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; case 4: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; case 8: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; default: __bad_size_call_parameter(); break; } pscr_ret__; }) == ((c->freelist)) && ({ typeof(((s->cpu_slab->tid))) pscr_ret__; do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->tid)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->tid)))) { case 1: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; case 2: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; case 4: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; case 8: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; default: __bad_size_call_parameter(); break; } pscr_ret__; }) == ((tid))) { do { do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->freelist)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->freelist)))) { case 1: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((object)))); } while (0); break; case 2: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((object)))); } while (0); break; case 4: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((object)))); } while (0); break; case 8: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((object)))); } while (0); break; default: __bad_size_call_parameter(); break; } } while (0); do { do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->tid)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->tid)))) { case 1: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; case 2: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; case 4: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; case 8: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; default: __bad_size_call_parameter(); break; } } while (0); __ret = 1; } (__ret); }); do { do { ({ unsigned long __dummy; typeof(flags) __dummy2; (void)(&__dummy == &__dummy2); 1; }); arch_local_irq_restore(flags); } while (0); } while (0); ret__; }); break; case 4: pdcrb_ret__ = ({ int ret__; unsigned long flags; do { do { ({ unsigned long __dummy; typeof(flags) __dummy2; (void)(&__dummy == &__dummy2); 1; }); flags = arch_local_irq_save(); } while (0); } while (0); ret__ = ({ int __ret = 0; if (({ typeof(((s->cpu_slab->freelist))) pscr_ret__; do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->freelist)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->freelist)))) { case 1: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; case 2: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; case 4: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; case 8: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; default: __bad_size_call_parameter(); break; } pscr_ret__; }) == ((c->freelist)) && ({ typeof(((s->cpu_slab->tid))) pscr_ret__; do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->tid)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->tid)))) { case 1: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; case 2: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; case 4: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; case 8: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; default: __bad_size_call_parameter(); break; } pscr_ret__; }) == ((tid))) { do { do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->freelist)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->freelist)))) { case 1: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((object)))); } while (0); break; case 2: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((object)))); } while (0); break; case 4: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((object)))); } while (0); break; case 8: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((object)))); } while (0); break; default: __bad_size_call_parameter(); break; } } while (0); do { do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->tid)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->tid)))) { case 1: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; case 2: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; case 4: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; case 8: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; default: __bad_size_call_parameter(); break; } } while (0); __ret = 1; } (__ret); }); do { do { ({ unsigned long __dummy; typeof(flags) __dummy2; (void)(&__dummy == &__dummy2); 1; }); arch_local_irq_restore(flags); } while (0); } while (0); ret__; }); break; case 8: pdcrb_ret__ = ({ int ret__; unsigned long flags; do { do { ({ unsigned long __dummy; typeof(flags) __dummy2; (void)(&__dummy == &__dummy2); 1; }); flags = arch_local_irq_save(); } while (0); } while (0); ret__ = ({ int __ret = 0; if (({ typeof(((s->cpu_slab->freelist))) pscr_ret__; do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->freelist)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->freelist)))) { case 1: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; case 2: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; case 4: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; case 8: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->freelist)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->freelist)))))) *)((&(((s->cpu_slab->freelist))))); }); })); break; default: __bad_size_call_parameter(); break; } pscr_ret__; }) == ((c->freelist)) && ({ typeof(((s->cpu_slab->tid))) pscr_ret__; do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->tid)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->tid)))) { case 1: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; case 2: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; case 4: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; case 8: pscr_ret__ = (*({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&(((s->cpu_slab->tid)))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&(((s->cpu_slab->tid)))))) *)((&(((s->cpu_slab->tid))))); }); })); break; default: __bad_size_call_parameter(); break; } pscr_ret__; }) == ((tid))) { do { do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->freelist)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->freelist)))) { case 1: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((object)))); } while (0); break; case 2: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((object)))); } while (0); break; case 4: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((object)))); } while (0); break; case 8: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->freelist))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->freelist))))))) *)((&((((s->cpu_slab->freelist)))))); }); }) = ((((object)))); } while (0); break; default: __bad_size_call_parameter(); break; } } while (0); do { do { const void *__vpp_verify = (typeof(&(((s->cpu_slab->tid)))))((void*)0); (void) __vpp_verify; } while (0); switch(sizeof(((s->cpu_slab->tid)))) { case 1: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; case 2: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; case 4: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; case 8: do { *({ (void)(0); ({ do { const void *__vpp_verify = (typeof(((&((((s->cpu_slab->tid))))))))((void*)0); (void) __vpp_verify; } while (0); (typeof(*((&((((s->cpu_slab->tid))))))) *)((&((((s->cpu_slab->tid)))))); }); }) = ((((next_tid(tid))))); } while (0); break; default: __bad_size_call_parameter(); break; } } while (0); __ret = 1; } (__ret); }); do { do { ({ unsigned long __dummy; typeof(flags) __dummy2; (void)(&__dummy == &__dummy2); 1; }); arch_local_irq_restore(flags); } while (0); } while (0); ret__; }); break; default: __bad_size_call_parameter(); break; } pdcrb_ret__; })) {

            goto redo;
  }

 } else
  __slab_free(s, page, x, addr);
}

void kfree(const void *x)
{
    struct page *page;
 void *object = (void *)x;



 if (((unsigned long)(x) <= (unsigned long)((void *)16)))
  return;

 page = virt_to_head_page(x);
 if (!PageSlab(page)) {

  put_page(page);
  return;
 }
 slab_free(page->slab, page, object, (unsigned long)__builtin_return_address(0));
}


void kmem_cache_free(struct kmem_cache *s, void *x)
{
    struct page *page;
 page = virt_to_head_page(x);
 slab_free(s, page, x, (unsigned long)__builtin_return_address(0));

}

static struct kmem_cache *kmem_cache_node;

static void free_kmem_cache_nodes(struct kmem_cache *s)
{
    int node;

 for ((node) = 0; (node) == 0; (node) = 1) {
        struct kmem_cache_node *n = s->node[node];

  if (n)
   kmem_cache_free(kmem_cache_node, n);
  s->node[node] = ((void*)0);
 }
}


static void early_kmem_cache_node_alloc(int node)
{
    struct page *page;
 struct kmem_cache_node *n;

 page = new_slab(kmem_cache_node, ((((gfp_t)0x20u)) & ~((gfp_t)0x20u)), node);

 if (page_to_nid(page) != node) {

 }

 n = page->freelist;
 page->freelist = get_freepointer(kmem_cache_node, n);
 page->inuse++;
 page->frozen = 0;
 kmem_cache_node->node[node] = n;

 init_kmem_cache_node(n, kmem_cache_node);
 inc_slabs_node(kmem_cache_node, node, page->objects);

 add_partial(n, page, 0);
}


static int init_kmem_cache_nodes(struct kmem_cache *s)
{
    int node;

 for ((node) = 0; (node) == 0; (node) = 1) {
        struct kmem_cache_node *n;

  if (slab_state == DOWN) {
            early_kmem_cache_node_alloc(node);
   continue;
  }

  n = kmem_cache_alloc_node(kmem_cache_node, (((gfp_t)0x10u) | ((gfp_t)0x40u) | ((gfp_t)0x80u)), node);

  if (!n) {
            free_kmem_cache_nodes(s);
   return 0;
  }

  s->node[node] = n;
  init_kmem_cache_node(n, s);
 }
 return 1;
}

static int kmem_cache_open(struct kmem_cache *s, const char *name, size_t size,
                     size_t align, unsigned long flags, void (*ctor)(void *))
{
    memset(s, 0, kmem_size);
 s->name = name;
 s->ctor = ctor;
 s->objsize = size;
 s->align = align;
 s->flags = kmem_cache_flags(size, flags, name, ctor);
 s->reserved = 0;

    if ((sizeof(((struct page *)((void*)0))->lru) < sizeof(struct rcu_head)) && (s->flags & 0x00080000UL))
  s->reserved = sizeof(struct rcu_head);

 if (!calculate_sizes(s, -1))
  goto error;

 if (0) {
        if (get_order(s->size) > get_order(s->objsize)) {
            s->flags &= ~(0x00000400UL | 0x00000800UL | 0x00010000UL);
   s->offset = 0;
   if (!calculate_sizes(s, -1))
    goto error;
  }
 }


 if ((1) && (s->flags & (0x00000400UL | 0x00000800UL | 0x00010000UL | 0x00200000UL | 0x00000100UL)) == 0)
  s->flags |= 0x40000000UL;

    set_min_partial(s, ( __builtin_constant_p(s->size) ? ( (s->size) < 1 ? ____ilog2_NaN() : (s->size) & (1ULL << 63) ? 63 : (s->size) & (1ULL << 62) ? 62 : (s->size) & (1ULL << 61) ? 61 : (s->size) & (1ULL << 60) ? 60 : (s->size) & (1ULL << 59) ? 59 : (s->size) & (1ULL << 58) ? 58 : (s->size) & (1ULL << 57) ? 57 : (s->size) & (1ULL << 56) ? 56 : (s->size) & (1ULL << 55) ? 55 : (s->size) & (1ULL << 54) ? 54 : (s->size) & (1ULL << 53) ? 53 : (s->size) & (1ULL << 52) ? 52 : (s->size) & (1ULL << 51) ? 51 : (s->size) & (1ULL << 50) ? 50 : (s->size) & (1ULL << 49) ? 49 : (s->size) & (1ULL << 48) ? 48 : (s->size) & (1ULL << 47) ? 47 : (s->size) & (1ULL << 46) ? 46 : (s->size) & (1ULL << 45) ? 45 : (s->size) & (1ULL << 44) ? 44 : (s->size) & (1ULL << 43) ? 43 : (s->size) & (1ULL << 42) ? 42 : (s->size) & (1ULL << 41) ? 41 : (s->size) & (1ULL << 40) ? 40 : (s->size) & (1ULL << 39) ? 39 : (s->size) & (1ULL << 38) ? 38 : (s->size) & (1ULL << 37) ? 37 : (s->size) & (1ULL << 36) ? 36 : (s->size) & (1ULL << 35) ? 35 : (s->size) & (1ULL << 34) ? 34 : (s->size) & (1ULL << 33) ? 33 : (s->size) & (1ULL << 32) ? 32 : (s->size) & (1ULL << 31) ? 31 : (s->size) & (1ULL << 30) ? 30 : (s->size) & (1ULL << 29) ? 29 : (s->size) & (1ULL << 28) ? 28 : (s->size) & (1ULL << 27) ? 27 : (s->size) & (1ULL << 26) ? 26 : (s->size) & (1ULL << 25) ? 25 : (s->size) & (1ULL << 24) ? 24 : (s->size) & (1ULL << 23) ? 23 : (s->size) & (1ULL << 22) ? 22 : (s->size) & (1ULL << 21) ? 21 : (s->size) & (1ULL << 20) ? 20 : (s->size) & (1ULL << 19) ? 19 : (s->size) & (1ULL << 18) ? 18 : (s->size) & (1ULL << 17) ? 17 : (s->size) & (1ULL << 16) ? 16 : (s->size) & (1ULL << 15) ? 15 : (s->size) & (1ULL << 14) ? 14 : (s->size) & (1ULL << 13) ? 13 : (s->size) & (1ULL << 12) ? 12 : (s->size) & (1ULL << 11) ? 11 : (s->size) & (1ULL << 10) ? 10 : (s->size) & (1ULL << 9) ? 9 : (s->size) & (1ULL << 8) ? 8 : (s->size) & (1ULL << 7) ? 7 : (s->size) & (1ULL << 6) ? 6 : (s->size) & (1ULL << 5) ? 5 : (s->size) & (1ULL << 4) ? 4 : (s->size) & (1ULL << 3) ? 3 : (s->size) & (1ULL << 2) ? 2 : (s->size) & (1ULL << 1) ? 1 : (s->size) & (1ULL << 0) ? 0 : ____ilog2_NaN() ) : (sizeof(s->size) <= 4) ? __ilog2_u32(s->size) : __ilog2_u64(s->size) ));
 s->refcount = 1;

 if (!init_kmem_cache_nodes(s))
  goto error;

 if (alloc_kmem_cache_cpus(s))
  return 1;

 free_kmem_cache_nodes(s);
error:
 if (flags & 0x00040000UL)
  panic("Cannot create slab");

 return 0;
}

static void kmem_cache_bootstrap_fixup(struct kmem_cache *s)
{
    int node;
 list_add(&s->list, &slab_caches);
 s->refcount = -1;

 for ((node) = 0; (node) == 0; (node) = 1) {
        struct kmem_cache_node *n = get_node(s, node);
  struct page *p;

  if (n) {
            for (p = ({ const typeof(((typeof(*p) *)0)->lru) * __mptr = ((&n->partial)->next); (typeof(*p) *)((char *)__mptr - ((size_t) &((typeof(*p) *)0)->lru));}); &p->lru != (&n->partial); p = ({ const typeof(((typeof(*p) *)0)->lru) * __mptr = (p->lru.next); (typeof(*p) *)((char *)__mptr - ((size_t) &((typeof(*p) *)0)->lru));}))
    p->slab = s;
  }
 }
}

static struct kmem_cache *create_kmalloc_cache(const char *name, int size, unsigned int flags)
{
    struct kmem_cache *s;
 s = kmem_cache_alloc(kmem_cache, ((((gfp_t)0x20u)) & ~((gfp_t)0x20u)));

 if (!kmem_cache_open(s, name, size, __alignof__(unsigned long long), flags, ((void*)0)))
  goto panic;
 list_add(&s->list, &slab_caches);
 return s;
panic:
 panic("Creation of kmalloc slab");
 return ((void*)0);
}

void kmem_cache_init(void)
{
    int i;
 int caches = 0;
 struct kmem_cache *temp_kmem_cache;
 int order;
 struct kmem_cache *temp_kmem_cache_node;
 unsigned long kmalloc_size;

 kmem_size = ((size_t) &((struct kmem_cache *)0)->node) +
  1 * sizeof(struct kmem_cache_node *);

 kmalloc_size = (((kmem_size) + ((typeof(kmem_size))((1 << 5)) - 1)) & ~((typeof(kmem_size))((1 << 5)) - 1));
 order = get_order(2 * kmalloc_size);
 kmem_cache = (void *)__get_free_pages(((((gfp_t)0x20u)) & ~((gfp_t)0x20u)), order);

 kmem_cache_node = (void *)kmem_cache + kmalloc_size;

 kmem_cache_open(kmem_cache_node, "kmem_cache_node",
           sizeof(struct kmem_cache_node), 0,
           0x00002000UL | 0x00040000UL, ((void*)0));



    slab_state = PARTIAL;
 temp_kmem_cache = kmem_cache;

 kmem_cache_open(kmem_cache, "kmem_cache", kmem_size, 0,
           0x00002000UL | 0x00040000UL, ((void*)0));
 kmem_cache = kmem_cache_alloc(kmem_cache, ((((gfp_t)0x20u)) & ~((gfp_t)0x20u)));
 memcpy(kmem_cache, temp_kmem_cache, kmem_size);

 temp_kmem_cache_node = kmem_cache_node;

 kmem_cache_node = kmem_cache_alloc(kmem_cache, ((((gfp_t)0x20u)) & ~((gfp_t)0x20u)));
 memcpy(kmem_cache_node, temp_kmem_cache_node, kmem_size);

    kmem_cache_bootstrap_fixup(kmem_cache_node);
    caches++;
 kmem_cache_bootstrap_fixup(kmem_cache);
 caches++;

 free_pages((unsigned long)temp_kmem_cache, order);

 for (i = 8; i < 8; i += 8) {
        int elem = size_index_elem(i);
  if (elem >= (sizeof(size_index)/sizeof((size_index)[0])))
   break;
  size_index[elem] = 3;
 }

 if (8 == 64) {
        for (i = 64 + 8; i <= 96; i += 8)
   size_index[size_index_elem(i)] = 7;
 } else if (8 == 128) {
        for (i = 128 + 8; i <= 192; i += 8)
   size_index[size_index_elem(i)] = 8;
 }

 if (8 <= 32) {
        kmalloc_caches[1] = create_kmalloc_cache("kmalloc-96", 96, 0);
  caches++;
 }

 if (8 <= 64) {
        kmalloc_caches[2] = create_kmalloc_cache("kmalloc-192", 192, 0);
  caches++;
 }

 for (i = 3; i < (12 + 2); i++) {
        kmalloc_caches[i] = create_kmalloc_cache("kmalloc", 1 << i, 0);
  caches++;
 }

 slab_state = UP;

 if (8 <= 32) {

        kmalloc_caches[1]->name = ((void*)0);
 }

 if (8 <= 64) {

        kmalloc_caches[2]->name = ((void*)0);
 }

 for (i = 3; i < (12 + 2); i++) {





 }

 for (i = 0; i < (12 + 2); i++) {
        struct kmem_cache *s = kmalloc_caches[i];

  if (s && s->size) {

   char *name = "dma-kmalloc";
            kmalloc_dma_caches[i] = create_kmalloc_cache(name, s->objsize,
                                          0x00004000UL);
  }
 }
}
