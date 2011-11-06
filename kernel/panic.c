#include <Xc/kernel.h>

void panic(const char *format, ...)
{
    while(1);
}

void early_panic(const char *str)
{
    early_print_str(str);
	while(1);
}
