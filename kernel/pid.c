#include <Xc/init.h>
#include <Xc/types.h>
#include <Xc/bootmem.h>
#include <stddef.h>

int pid_max = PID_MAX_DEFAULT;

#define RESERVED_PIDS 300

int pid_max_min = RESERVED_PIDS + 1;
int pid_max_max = PID_MAX_LIMIT;

static struct hlist_head *pid_hash;
static unsigned int pidhash_shift = 4;

void __init pidhash_init(void)
{
    int i, pidhash_size;

	pid_hash = alloc_large_system_hash("PID", sizeof(*pid_hash), 0, 18, 
			                           HASH_EARLY | HASH_SMALL, &pidhash_shift, NULL, 4096);    pidhash_size = 1 << pidhash_shift;

	for (i = 0; i < pidhash_size; i++)
		INIT_HLIST_HEAD(&pid_hash[i]);
}

void __init pidmap_init(void)
{
    (void)pid_max;
	(void)pid_max_min;
}
