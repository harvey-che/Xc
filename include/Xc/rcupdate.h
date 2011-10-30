#ifndef _XC_RCUPDATE_H
#define _XC_RCUPDATE_H

struct rcu_head {
    struct rcu_head *next;
	void (*func)(struct rcu_head *head);
};

#endif 
