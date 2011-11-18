#ifndef _XC_NODEMASK_H
#define _XC_NODEMASK_H

#include <Xc/types.h>
#include <Xc/numa.h>

/* CONFIG_HIGHMEM is not defined */
enum node_states {
    N_POSSIBLE,
	N_ONLINE,
	N_NORMAL_MEMORY,
	N_HIGH_MEMORY = N_NORMAL_MEMORY,
	N_CPU,
	NR_NODE_STATES
};
/*
 * typedef struct { unsigned long bits[1]; } nodemask_t;
 */
typedef struct { DECLARE_BITMAP(bits, MAX_NUMNODES); } nodemask_t;

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

#define first_online_node 0
#define next_online_node(nid) (MAX_NUMNODES)

#define for_each_node_state(node, __state)  \
	for ((node) = 0; (node) == 0; (node) = 1)

#define node_online(node) node_state((node), N_ONLINE)
#define for_each_online_node(node) for_each_node_state(node, N_ONLINE)

#define nr_node_ids 1
#define nr_online_nodes 1
#define node_isset(node, nodemask) test_bit((node), (nodemask).bits)

#define NODE_MASK_LAST_WORD BITMAP_LAST_WORD_MASK(MAX_NUMNODES)
#define NODE_MASK_ALL    \
	((nodemask_t) { {    \
	     [BITS_TO_LONGS(MAX_NUMNODES) - 1] = NODE_MASK_LAST_WORD   \
	 } })

#endif

