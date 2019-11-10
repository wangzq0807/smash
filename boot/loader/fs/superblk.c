#include "superblk.h"
#include "disk_drv.h"
#include "asm.h"
#include "string.h"
#include "log.h"

#define PARTION_NUM     (4)
#define PARTION_POS     (446)
#define BOOT_FLAG       (0xaa55)
#define BOOT_FLAG_POS   (510)
#define ACTIVE_FLAG     (0x80)

#define SUPER_BLOCK_BEGAIN      1           // 超级块在分区中的开始块号
#define SUPER_BLOCK_SIZE        1           // 一个超级块占的块数

PartionEntity partion_table[PARTION_NUM];
static SuperBlock super_blk[4];

#define SECTORS_PER_BLOCK (BLOCK_SIZE / SECTOR_SIZE)
#define PH 255
#define PS 63

// 读取分区信息
int
read_partion_entity()
{
    // 获取磁盘上第一个块
    void *block_buf = ata_read(0);
    // 读取可启动标志
    uint16_t bootable = *(uint16_t*)(block_buf + BOOT_FLAG_POS);
    if (bootable != BOOT_FLAG) {
        KLOG(ERROR, "not bootable! %d", bootable);
        return -1;
    }
    KLOG(DEBUG, "bootable");
    memcpy((void*)partion_table, block_buf + PARTION_POS, PARTION_NUM*sizeof(PartionEntity));
    return 0;
}

PartionEntity *
get_partion_entity()
{
    // NOTE : dev unused
    return &partion_table[0];
}

zone_t
get_super_block_pos()
{
    zone_t superblk_pos = 0;
    PartionEntity *entity = &partion_table[0];
    if (entity->pe_lba_start > 0) {
        const zone_t nstart = entity->pe_lba_start / SECTORS_PER_BLOCK;
        superblk_pos = nstart + SUPER_BLOCK_BEGAIN;
    }
    else {
        superblk_pos = entity->pe_start_cylinder * PH * PS
                    + entity->pe_start_header * PS
                    + entity->pe_start_sector - 1;
        superblk_pos = superblk_pos / SECTORS_PER_BLOCK + SUPER_BLOCK_BEGAIN;
    }
    return superblk_pos;
}

error_t
init_file_system()
{
    error_t ret = 0;
    // 读取磁盘分区信息
    ret = read_partion_entity();
    if (ret != 0)
        return ret;
    zone_t pos = get_super_block_pos();
    void *block_buf = ata_read(pos);
    memcpy(&super_blk[0], block_buf, sizeof(super_blk));
    if (super_blk[0].sb_magic != MINIX_V2 ) {
        ret = -1;
    }
    return ret;
}

const SuperBlock *
get_super_block()
{
    return &super_blk[0];
}
