#include "zones.h"
#include "inodes.h"
#include "buffer.h"
#include "superblk.h"
#include "partion.h"

#define PER_BLOCK_BYTES     (1 << BLOCK_LOG_SIZE)
#define PER_BLOCK_SECTORS   (PER_BLOCK_BYTES/SECTOR_SIZE)
// 间接索引块数
#define INDIRECT_ZONES      (PER_BLOCK_BYTES / sizeof(zone_t))
// 双间接索引
#define DINDIRECT_ZONES     (INDIRECT_ZONES * INDIRECT_ZONES)
// 三间接索引
#define TINDIRECT_ZONES     (INDIRECT_ZONES * INDIRECT_ZONES * INDIRECT_ZONES)

// TODO: znode_map数组的大小是不够用的
struct BlockBuffer *znode_map[MAX_ZMAP_NUM] = {0};

error_t
init_zones(dev_t dev)
{
    const blk_t superblk_begin = get_super_block_begin(dev);
    const struct SuperBlock *super_block = get_super_block(dev);
    const blk_t icnt = super_block->sb_imap_blocks;
    const blk_t zcnt = super_block->sb_zmap_blocks;

    const blk_t inode_begin = superblk_begin + SUPER_BLOCK_SIZE;
    const blk_t znode_pos = inode_begin + icnt;
    for (int i = 0; i < zcnt && i < MAX_ZMAP_NUM; ++i) {
        znode_map[i] = get_block(dev, znode_pos + i);
    }
    return 0;
}

static zone_t
_alloc_bitmap(struct BlockBuffer **node_map, blk_t cnt)
{
    for (blk_t blk = 0; blk < cnt; ++blk) {
        struct BlockBuffer *buffer = node_map[blk];
        for (int num = 0; num < PER_BLOCK_BYTES/sizeof(int); ++num) {
            int bits = sizeof(int) * 8;
            for (int bit = 0; bit < bits; ++bit) {
                if (_get_bit(((int *)buffer->bf_data)[num], bit) == 0) {
                    _set_bit(&((int *)buffer->bf_data)[num], bit);
                    buffer->bf_status |= BUF_DIRTY;
                    return ((blk * PER_BLOCK_BYTES) << 3) + num * bits + bit;
                }
            }
        }
    }
    return 0;
}

static error_t
_clear_bitmap(struct BlockBuffer **node_map, blk_t cnt)
{
    int blkbits = PER_BLOCK_BYTES << 3;
    blk_t num = cnt / blkbits;
    blk_t bits = cnt % blkbits;
    blk_t index = bits / sizeof(int);
    int bit = bits % sizeof(int);
    struct BlockBuffer *buffer = node_map[num];
    _clear_bit(&((int *)buffer->bf_data)[index], bit);
    return 0;
}

zone_t
alloc_zone(dev_t dev)
{
    const struct SuperBlock *super_block = get_super_block(dev);
    const blk_t zcnt = super_block->sb_zmap_blocks;
    zone_t znode = _alloc_bitmap(znode_map, zcnt);
    return znode;
}

error_t
delete_zone(dev_t dev, zone_t num)
{
    _clear_bitmap(znode_map, num);
    return 0;
}

zone_t
get_zone(struct IndexNode *inode, seek_t bytes_offset)
{
    struct PartionEntity *entity = get_partion_entity(inode->in_dev);
    const blk_t nstart = entity->pe_lba_start / PER_BLOCK_SECTORS;
    // TODO : 这里暂时假设zone和block大小相同
    blk_t block_num = 0;
    const zone_t zone_num = bytes_offset >> BLOCK_LOG_SIZE;
    // 直接索引
    if (zone_num < DIRECT_ZONE) {
        block_num = inode->in_inode.in_zones[zone_num];
    }
    // 间接索引
    else if (zone_num < (DIRECT_ZONE + INDIRECT_ZONES)) {
        const uint32_t in_blk = inode->in_inode.in_zones[DIRECT_ZONE];
        const zone_t in_total_num = zone_num - DIRECT_ZONE;
        struct BlockBuffer *buf = get_block(inode->in_dev, nstart+in_blk);
        block_num = ((uint32_t *)buf->bf_data)[in_total_num];
        release_block(buf);
    }
    // 双间接索引
    else if (zone_num < (DIRECT_ZONE + INDIRECT_ZONES + DINDIRECT_ZONES)) {
        uint32_t db_blk = inode->in_inode.in_zones[DIRECT_ZONE + 1];
        zone_t db_total_num = zone_num - DIRECT_ZONE - INDIRECT_ZONES;

        zone_t db_offset = db_total_num / INDIRECT_ZONES;
        zone_t db_inoffset = db_total_num % INDIRECT_ZONES;

        struct BlockBuffer *buf = get_block(inode->in_dev, nstart+db_blk);
        uint32_t inblock_num = ((uint32_t *)buf->bf_data)[db_offset];
        release_block(buf);

        buf = get_block(inode->in_dev, nstart+inblock_num);
        block_num = ((uint32_t *)buf->bf_data)[db_inoffset];
        release_block(buf);
    }
    // 三间接索引
    else if (zone_num < (DIRECT_ZONE + INDIRECT_ZONES + DINDIRECT_ZONES + TINDIRECT_ZONES)) {
        uint32_t tr_blk = inode->in_inode.in_zones[DIRECT_ZONE + 2];
        zone_t tr_total_num = zone_num - DIRECT_ZONE - INDIRECT_ZONES - DINDIRECT_ZONES;

        zone_t tr_offset = tr_total_num / DINDIRECT_ZONES;
        zone_t tr_inoffset = (tr_total_num % DINDIRECT_ZONES)/INDIRECT_ZONES;
        zone_t tr_dboffset = tr_total_num % INDIRECT_ZONES;

        struct BlockBuffer *buf = get_block(inode->in_dev, nstart+tr_blk);
        uint32_t inblock_num = ((uint32_t *)buf->bf_data)[tr_offset];
        release_block(buf);

        buf = get_block(inode->in_dev, nstart+inblock_num);
        uint32_t dbblock_num = ((uint32_t *)buf->bf_data)[tr_inoffset];
        release_block(buf);

        buf = get_block(inode->in_dev, nstart+dbblock_num);
        block_num = ((uint32_t *)buf->bf_data)[tr_dboffset];
        release_block(buf);
    }
    if (block_num != 0)
        return nstart + block_num;
    else
        return 0;
}

error_t
truncate_zones(struct IndexNode *inode)
{
    return 0;
}

static inline uint32_t
_get_znone_begin(dev_t dev)
{
    struct PartionEntity *entity = get_partion_entity(dev);
    const uint32_t nstart = entity->pe_lba_start / PER_BLOCK_SECTORS;
    const struct SuperBlock *super_block = get_super_block(dev);
    return nstart + super_block->sb_first_datazone;
}
