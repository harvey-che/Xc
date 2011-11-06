#ifndef _XC_RANGE_H
#define _XC_RANGE_H

#include <Xc/types.h>

struct range {
    u64 start;
	u64 end;
};

int add_range(struct range *range, int az, int nr_range, u64 start, u64 end);
void subtract_range(struct range *range, int az, u64 start, u64 end);
int clean_sort_range(struct range *range, int az);

#endif
