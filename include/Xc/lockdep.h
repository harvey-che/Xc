#ifndef _XC_LOCKDEP_H
#define _XC_LOCKDEP_H

/* Don't support Lockdep */
struct lock_class_key {};

#define lockdep_set_class(lock, key)    do {} while (0)

#endif
