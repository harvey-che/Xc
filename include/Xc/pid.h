#ifndef _XC_PID_H
#define _XC_PID_H

#include <Xc/types.h>

enum pid_type {
    PIDTYPE_PID,
	PIDTYPE_PGID,
	PIDTYPE_SID,
	PIDTYPE_MAX
};

struct upid {
    int nr;
	/* struct pid_namespace *ns; */
	struct hlist_node pid_chain;
};

struct pid {
    atomic_t count;
	unsigned int level;

	struct hlist_head tasks[PIDTYPE_MAX];
	/* struct rcu_head rcu; */
	struct upid numbers[1];
};

#endif
