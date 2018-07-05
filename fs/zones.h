#ifndef __ZONES_H__
#define __ZONES_H__
#include "defs.h"

struct IndexNode;

error_t
init_zones(dev_t dev);

zone_t
alloc_zone(dev_t dev);

error_t
delete_zone(dev_t dev, zone_t num);

zone_t
get_zone(struct IndexNode *inode, seek_t bytes_offset);

error_t
truncate_zones(struct IndexNode *inode);

#endif // __ZONES_H
