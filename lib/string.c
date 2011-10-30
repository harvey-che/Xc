#include <Xc/string.h>

void *memcpy(void *dest, const void *src, size_t count)
{
    char *tmp = dest;
	const char *s = src;

	while (count--)
		*tmp++ = *s++;
	return dest;
}

void *memmove(void *dest, const void *src, size_t count)
{
    char *tmp;
	const char *s;

	if (dest <= src) {
        tmp = dest;
		s =src;
		while (count--)
			*tmp++ = *s++;
	} else {
        tmp = dest;
		tmp += count;
		s = src;
		s += count;
		while (count--)
			*--tmp = *--s;
	}
	return dest;
}

void *memset(void *s, int c, size_t count)
{
    char *xs = s;
	while (count--)
		*xs++ = c;
	return s;
}
