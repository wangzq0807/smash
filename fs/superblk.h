#ifndef __SUPERBLK_H__
#define __SUPERBLK_H__
#include "sys/types.h"

// minix v2 文件系统
typedef struct _SuperBlock SuperBlock;
struct _SuperBlock {
    uint16_t    sb_inodes;              // i节点总数
    uint16_t    sb_nzones;              // （已废弃）
    int16_t     sb_imap_blocks;         // i节点位图所占的块数
    int16_t     sb_zmap_blocks;         // 区块位图所占的块数
    uint16_t    sb_first_datazone;      // 数据区的开始块号
    int16_t     sb_log_zone_size;       // log2 块/区块(废弃)
    uint32_t    sb_max_file_size;       // 最大文件长度
    int16_t     sb_magic;               // 文件系统魔数
    int16_t     sb_padding;             // （无用）
    uint32_t    sb_zones;               // 区块总数
};

error_t init_super_block(dev_t dev);

uint32_t get_super_block_begin(dev_t dev);

const SuperBlock * get_super_block(dev_t dev);

void dump_super_block(dev_t dev);

#endif // __SUPERBLK_H__