#ifndef _XC_COMPILER_GCC_H
#define _XC_COMPILER_GCC_H

#define __weak __attribute__((weak))
#define __aligned(x)    __attribute__((aligned(x)))

/* The "volatile" is due to gcc bugs */
#define barrier() __asm__ __volatile__("": : :"memory")

#define uninitialized_var(x) x = x

#endif
