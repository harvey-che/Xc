#include <Xc/gfp.h>
#include <Xc/string.h>

void *kmemdup(const void *src, size_t len, gfp_t gfp)
{
    void *p;
	p = kmalloc_track_caller(len, gfp);
	if (p)
		memcpy(p, src, len);
	return p;
}
