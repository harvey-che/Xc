#ifndef _XC_CURRENT_H
#define _XC_CURRENT_H

#include <Xc/thread_info.h>

#define get_current() (current_thread_info()->task)
#define current get_current()

#endif
