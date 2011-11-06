#ifndef _XC_CPUSET_H
#define _XC_CPUSET_H

static inline void get_mems_allowed(void)
{

}

static inline void put_mems_allowed(void)
{

}

static inline void cpuset_init_current_mems_allowed(void) {}

static inline int cpuset_zone_allowed_softwall(struct zone *z, gfp_t gfp_mask)
{
    return 1;
}

#define cpuset_current_mems_allowed (node_states[N_HIGH_MEMORY])

#endif
