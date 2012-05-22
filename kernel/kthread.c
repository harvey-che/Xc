#include <Xc/sched.h>
//#include <Xc/topology.h> is included in mmzone.h
#include <Xc/mmzone.h>

int tsk_fork_get_node(struct task_struct *tsk)
{
    return numa_node_id();
}
