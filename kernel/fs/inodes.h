#ifndef __NODES_H__
#define __NODES_H__
#include "sys/types.h"
#include "fsdefs.h"
#include "lib/list.h"
#include "lib/hashmap.h"

#define INODE_FREE          0
#define INODE_LOCK          1
#define INODE_DIRTY         2

typedef struct _PyIndexNode PyIndexNode;
struct _PyIndexNode {
    uint16_t    in_file_mode;      // 文件类型, 权限等,
    int16_t     in_num_links;      // 链接到这个文件的数量
    int16_t     in_owner_id;       // 拥有者的id
    int8_t      in_group_id;       // 所在组的id
    uint32_t    in_file_size;      // 文件大小 in types
    time_t      in_atime;          // 访问时间
    time_t      in_mtime;          // 文件修改时间
    time_t      in_ctime;          // Inode修改时间
    uint32_t    in_zones[NUMBER_ZONE];      // 区块
};

typedef struct _IndexNode IndexNode;
struct _IndexNode {
    PyIndexNode         in_inode;       // 磁盘上inode内容
    dev_t               in_dev;         // 设备号
    uint32_t            in_status;      // 状态
    ino_t               in_inum;        // inode编号
    uint16_t            in_refs;        // 引用计数
    ListNode            in_link;
    HashNode            in_hashnode;  // hash表
};

error_t
init_inodes(dev_t dev);

IndexNode *
alloc_inode(dev_t dev);

error_t
delete_inode(IndexNode *inode);

IndexNode *
get_inode(dev_t dev, uint16_t idx);

void
release_inode(IndexNode *inode);

void
sync_inodes(dev_t dev);

static inline void
_set_bit(int *byte, int num)
{
    const int val = 1 << num;
    *byte |= val;
}

static inline void
_clear_bit(int *byte, int num)
{
    const int val = 1 << num;
    *byte &= ~val;
}

static inline int
_get_bit(int byte, int num)
{
    const int val = 1 << num;
    return byte & val;
}

#endif // __NODES_H__