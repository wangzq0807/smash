#ifndef __ZONES_H__
#define __ZONES_H__
#include "defs.h"

struct IndexNode;

error_t
init_zones(uint16_t dev);

uint32_t
alloc_zone(uint16_t dev);

uint32_t
get_zone(struct IndexNode *inode, uint32_t bytes_offset, uint32_t *offset_in_blk);

#endif // __ZONES_H
