#ifndef _XC_PFN_H
#define _XC_PFN_H

#include <Xc/types.h>

#define PFN_PHYS(x) ((phys_addr_t)(x) << PAGE_SHIFT)
#define PFN_DOWN(x) ((x) >> PAGE_SHIFT)
#define PFN_UP(x) (((x) + PAGE_SIZE - 1) >> PAGE_SHIFT)

#endif
