#ifndef __NODES_H__
#define __NODES_H__
#include "types.h"
#include "superblk.h"

// 间接索引块数
#define INDIRECT_ZONES      (BLOCK_SIZE / sizeof(zone_t))
// 双间接索引
#define DINDIRECT_ZONES     (INDIRECT_ZONES * INDIRECT_ZONES)
// 三间接索引
#define TINDIRECT_ZONES     (INDIRECT_ZONES * INDIRECT_ZONES * INDIRECT_ZONES)

typedef struct {
    uint16_t    in_file_mode;      // 文件类型, 权限等,
    int16_t     in_num_links;      // 链接到这个文件的数量
    int16_t     in_owner_id;       // 拥有者的id
    int8_t      in_group_id;       // 所在组的id
    uint32_t    in_file_size;      // 文件大小 in types
    time_t      in_atime;          // 访问时间
    time_t      in_mtime;          // 文件修改时间
    time_t      in_ctime;          // Inode修改时间
    uint32_t    in_zones[NUMBER_ZONE];      // 区块
} IndexNode;

IndexNode *
get_inode(ino_t inode_index);

zone_t
get_zone(const IndexNode *inode, off_t bytes_offset);

static inline int
_get_bit(uint8_t byte, int num)
{
    const int val = 1 << num ;
    return byte & val;
}

#endif // __NODES_H__