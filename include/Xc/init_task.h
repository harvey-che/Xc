#ifndef _XC_INIT_TASK_H
#define _XC_INIT_TASK_H

#define INIT_TASK(tsk)    \
{                         \
	.state = 0,     \
	.stack = &init_thread_info, \
}

#endif
