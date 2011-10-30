#ifndef _XC_COMPILER_GCC_H
#define _XC_COMPILER_GCC_H

/* The "volatile" is due to gcc bugs */
#define barrier() __asm__ __volatile__("": : :"memory")

#define uninitialized_var(x) x = x

#endif
