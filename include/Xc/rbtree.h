#ifndef _XC_RBTREE_H
#define _XC_RBTREE_H

struct rb_node {
    unsigned long rb_parent_color;
#define RB_RED   0
#define RB_BLACK 1
	struct rb_node *rb_right;
	struct rb_node *rb_left;
} __attribute__((aligned(sizeof(long))));

struct rb_root {
    struct rb_node *rb_node;
};

#define rb_entry(ptr, type, member) container_of(ptr, type, member)

static inline void rb_link_node(struct rb_node *node, struct rb_node *parent, 
		                        struct rb_node **rb_link)
{
    node->rb_parent_color = (unsigned long)parent;
	node->rb_left = node->rb_right = NULL;

	*rb_link = node;
}

#endif
