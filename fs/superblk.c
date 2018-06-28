#include "superblk.h"
#include "hard_disk.h"
#include "asm.h"
#include "string.h"
#include "partion.h"
#include "log.h"
#include "buffer.h"
#include "fs.h"
#include "partion.h"

static struct SuperBlock super_blk[4];

#define PER_BLOCK_SECTORS   ((1<<BLOCK_LOG_SIZE) / SECTOR_SIZE);

// NOTE : dev unused
uint32_t
get_super_block_begin(uint16_t dev)
{
    struct PartionEntity *entity = get_partion_entity(dev);
    const uint32_t nstart = entity->pe_lba_start / PER_BLOCK_SECTORS;
    const uint32_t superblk_pos = nstart + SUPER_BLOCK_BEGAIN;
    return superblk_pos;
}

// NOTE : dev unused
error_t
init_super_block(uint16_t dev)
{
    error_t ret = 0;
    uint32_t pos = get_super_block_begin(dev);
    struct BlockBuffer *buffer = get_block(dev, pos);
    memcpy(&super_blk[0], buffer->bf_data, sizeof(super_blk));
    if (super_blk[0].sb_magic != MINIX_V2 ) {
        print("only minifs v2 is supported\n");
        ret = -1;
    }
    release_block(buffer);
    return ret;
}

// NOTE : dev unused
const struct SuperBlock *
get_super_block(uint16_t dev)
{
    return &super_blk[0];
}

void
dump_super_block(uint16_t dev)
{
    const struct SuperBlock *sb = get_super_block(dev);
    print("sb_magic"); printx(sb->sb_magic); print("\n");
    print("sb_inodes"); printx(sb->sb_inodes); print("\n");
    print("sb_zones"); printx(sb->sb_zones); print("\n");
    print("sb_imap_blocks"); printx(sb->sb_imap_blocks); print("\n");
    print("sb_zmap_blocks"); printx(sb->sb_zmap_blocks); print("\n");
    print("sb_first_datazone"); printx(sb->sb_first_datazone); print("\n");
    print("sb_log_zone_size"); printx(sb->sb_log_zone_size); print("\n");
}