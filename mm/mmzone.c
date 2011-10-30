#include <Xc/mmzone.h>

static inline int zref_in_nodemask(struct zoneref *zref, nodemask_t *nodes)
{
	/* CONFIG_NUMA is not defined */
    return 1;
}

struct zoneref *next_zones_zonelist(struct zoneref *z, enum zone_type highest_zoneidx, nodemask_t *nodes, 
		struct zone **zone)
{
    if (nodes == NULL)
		while (zonelist_zone_idx(z) > highest_zoneidx)
			z++;
	else
		while (zonelist_zone_idx(z) > highest_zoneidx || 
				(z->zone && !zref_in_nodemask(z, nodes)))
			z++;
	*zone = zonelist_zone(z);
	return z;
}
