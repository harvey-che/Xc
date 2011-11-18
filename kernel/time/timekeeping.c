#include <Xc/jiffies.h>
#include <Xc/kernel.h>

static int counter_in_timer = 0;
static char buf_in_timer[16];
void do_timer(unsigned long ticks)
{
    jiffies_64 += ticks;
	early_print_str(itoah(buf_in_timer, counter_in_timer++));
}
