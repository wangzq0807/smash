#include "superblk.h"
#include "hard_disk.h"
#include "asm.h"
#include "string.h"
#include "partion.h"
#include "log.h"
#include "buffer.h"
#include "fsdefs.h"
#include "partion.h"

static SuperBlock super_blk[4];

#define SECTORS_PER_BLOCK (BLOCK_SIZE / SECTOR_SIZE)
#define PH 255
#define PS 63
// NOTE : dev unused
zone_t
get_super_block_begin(dev_t dev)
{
    zone_t superblk_pos = 0;
    PartionEntity *entity = get_partion_entity(dev);
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

// NOTE : dev unused
error_t
init_super_block(dev_t dev)
{
    error_t ret = 0;
    zone_t pos = get_super_block_begin(dev);
    BlockBuffer *buffer = get_block(dev, pos);
    memcpy(&super_blk[0], buffer->bf_data, sizeof(super_blk));
    if (super_blk[0].sb_magic != MINIX_V2 ) {
        printk("only minifs v2 is supported\n");
        ret = -1;
    }
    release_block(buffer);
    return ret;
}

// NOTE : dev unused
const SuperBlock *
get_super_block(dev_t dev)
{
    return &super_blk[0];
}

void
dump_super_block(dev_t dev)
{
    const SuperBlock *sb = get_super_block(dev);
    printk("sb_magic %x\n", sb->sb_magic);
    printk("sb_inodes %x\n", sb->sb_inodes);
    printk("sb_zones %x\n", sb->sb_zones);
    printk("sb_imap_blocks %x\n", sb->sb_imap_blocks);
    printk("sb_zmap_blocks %x\n", sb->sb_zmap_blocks);
    printk("sb_first_datazone %x\n", sb->sb_first_datazone);
    printk("sb_log_zone_size %x", sb->sb_log_zone_size);
}