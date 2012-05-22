#ifndef _XC_TIMERQUEUE_H
#define _XC_TIMERQUEUE_H

#include <Xc/rbtree.h>

struct timerqueue_node {
	struct rb_node node;
	ktime_t expires;
};

struct timerqueue_head {
	struct rb_root head;
	struct timerqueue_node *next;
};

#endif
