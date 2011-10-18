#ifndef _XC_KERNEL_UTILITIES_H
#define _XC_KERNEL_UTILITIES_H

int printk(const char *format, ...);
void panic(const char *format, ...);

#define roundup(x, y) (                 \
{                                       \
	const typeof(y) __y = y;            \
	(((x) + (__y - 1)) / __y) * __y;    \
}                                       \
)
#endif
