#ifndef __SUPERBLK_H__
#define __SUPERBLK_H__
#include "sys/types.h"

#define MINIX_V2                0x2478      // minix v2文件系统标识
#define SUPER_BLOCK_SIZE        1           // 一个超级块占的块数

#define MAX_IMAP_NUM            8           // inode位图最大区块数
#define MAX_ZMAP_NUM            256         // znode位图最大区块数

#define DIRECT_ZONE             7           // 直接索引区块数
#define NUMBER_ZONE             10          // 总的索引区块数
#define FILENAME_LEN            29          // 文件名长度

#define ROOT_INODE              1           // 根节点
#define INVALID_ZONE            0
#define INVALID_INODE           0

typedef struct {
    uint8_t     pe_indicator;
    uint8_t     pe_start_header;
    uint8_t     pe_start_sector;
    uint8_t     pe_start_cylinder;
    uint8_t     pe_system_id;
    uint8_t     pe_end_header;
    uint8_t     pe_end_sector;
    uint8_t     pe_end_cylinder;
    uint32_t    pe_lba_start;
    uint32_t    pe_total_sector;
} PartionEntity;

// minix v2 文件系统
typedef struct {
    ino_t       sb_inodes;              // i节点总数
    uint16_t    sb_nzones;              // （已废弃）
    int16_t     sb_imap_blocks;         // i节点位图所占的块数
    int16_t     sb_zmap_blocks;         // 区块位图所占的块数
    uint16_t    sb_first_datazone;      // 数据区的开始块号
    int16_t     sb_log_zone_size;       // log2 块/区块(废弃)
    uint32_t    sb_max_file_size;       // 最大文件长度
    int16_t     sb_magic;               // 文件系统魔数
    int16_t     sb_padding;             // （无用）
    zone_t      sb_zones;               // 区块总数
} SuperBlock;

PartionEntity *get_partion_entity();

// 读取第一个分区的超级块
error_t init_file_system();

zone_t get_super_block_pos();

// 获取第一个分区的超级块
const SuperBlock * get_super_block();

#endif // __SUPERBLK_H__