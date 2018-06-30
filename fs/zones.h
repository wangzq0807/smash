#ifndef __ZONES_H__
#define __ZONES_H__
#include "defs.h"

struct IndexNode;

error_t
init_zones(dev_t dev);

zone_t
alloc_zone(dev_t dev);

blk_t
get_zone(struct IndexNode *inode, seek_t bytes_offset);

#endif // __ZONES_H
