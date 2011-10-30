#ifndef _XC_LIST_H
#define _XC_LIST_H

#include <Xc/types.h>

#define LIST_POISON1 ((void*)0x00100100)
#define LIST_POISON2 ((void*)0x00200200)

#define LIST_HEAD_INIT(name) {&(name), &(name)}

#define LIST_HEAD(name)   \
	struct list_head name = LIST_HEAD_INIT(name)

#define list_for_each_entry(pos, head, member)    \
	for (pos = list_entry((head)->next, typeof(*pos), member);    \
		 &pos->member != (head);     \
		 pos = list_entry(pos->member.next, typeof(*pos), member))

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
	entry->next = LIST_POISON1;
	entry->prev = LIST_POISON2;
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

static inline int list_empty(const struct list_head *head)
{
    return head->next == head;
}

static inline void list_move(struct list_head *list, struct list_head *head)
{
    __list_del_entry(list);
	list_add(list, head);
}

#define list_entry(ptr, type, member)    \
	container_of(ptr, type, member)
#endif
