#ifndef _XC_SPINLOCK_TYPES_H
#define _XC_SPINLOCK_TYPES_H

/* spinlock_types_up.h Don't support SMP */
/* debug version */
typedef struct {
    volatile unsigned int slock;
} arch_spinlock_t;

#define __ARCH_SPIN_LOCK_UNLOCKED {1}
/* end -- spinlock_types_up.h */

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


#define SPINLOCK_MAGIC 0xdead4ead
#define SPINLOCK_OWNER_INIT ((void*)-1L)
#define SPIN_DEBUG_INIT(lockname)     \
	.magic = SPINLOCK_MAGIC,          \
    .owner_cpu = -1,                  \
    .owner = SPINLOCK_OWNER_INIT,

#define SPIN_DEP_MAP_INIT(lockname)

#define __RAW_SPIN_LOCK_INITIALIZER(lockname)    \
    {                                            \
		.raw_lock = __ARCH_SPIN_LOCK_UNLOCKED,   \
		SPIN_DEBUG_INIT(lockname)                \
		SPIN_DEP_MAP_INIT(lockname) }

/*
 * (raw_spinlock_t) { .raw_lock = {1},
 *                    .magic = 0xdead4ead,
 *                    .owner_cpu = -1
 *                    .owner = ((void*)-1L) }
 */
#define __RAW_SPIN_LOCK_UNLOCKED(lockname)     \
	(raw_spinlock_t) __RAW_SPIN_LOCK_INITIALIZER(lockname)
#endif
