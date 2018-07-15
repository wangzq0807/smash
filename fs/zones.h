#ifndef __ZONES_H__
#define __ZONES_H__
#include "sys/types.h"

typedef struct _IndexNode IndexNode;

error_t
init_zones(dev_t dev);

zone_t
alloc_zone(IndexNode *inode);

error_t
delete_zone(dev_t dev, zone_t num);

zone_t
get_zone(IndexNode *inode, seek_t bytes_offset);

error_t
truncate_zones(IndexNode *inode);

#endif // __ZONES_H
