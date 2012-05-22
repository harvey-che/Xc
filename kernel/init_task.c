#include <Xc/init_task.h>
#include <Xc/thread_info.h>
#include <Xc/sched.h>
#include <Xc/percpu.h>

struct task_struct init_task = INIT_TASK(init_task);

union thread_union init_thread_union = {INIT_THREAD_INFO(init_task)};


DEFINE_PER_CPU_SHARED_ALIGNED(struct tss_struct, init_tss) = INIT_TSS;
