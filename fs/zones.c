#include "zones.h"
#include "inodes.h"
#include "buffer.h"
#include "superblk.h"
#include "partion.h"

#define PER_BLOCK_BYTES     (1 << BLOCK_LOG_SIZE)
#define PER_BLOCK_SECTORS   (PER_BLOCK_BYTES/SECTOR_SIZE)
// 间接索引块数
#define INDIRECT_ZONES      (PER_BLOCK_BYTES / sizeof(uint32_t))
// 双间接索引
#define DINDIRECT_ZONES     (INDIRECT_ZONES * INDIRECT_ZONES)
// 三间接索引
#define TINDIRECT_ZONES     (INDIRECT_ZONES * INDIRECT_ZONES * INDIRECT_ZONES)

// TODO: znode_map数组的大小是不够用的
struct BlockBuffer *znode_map[MAX_ZMAP_NUM] = {0};

error_t
init_zones(uint16_t dev)
{
    const uint32_t superblk_begin = get_super_block_begin(dev);
    const struct SuperBlock *super_block = get_super_block(dev);
    const uint32_t icnt = super_block->sb_imap_blocks;
    const uint32_t zcnt = super_block->sb_zmap_blocks;

    const uint32_t inode_begin = superblk_begin + SUPER_BLOCK_SIZE;
    const uint32_t znode_pos = inode_begin + icnt;
    for (int i = 0; i < zcnt && i < MAX_ZMAP_NUM; ++i) {
        znode_map[i] = get_block(dev, znode_pos + i);
    }
    return 0;
}

static uint32_t
_alloc_bit(struct BlockBuffer **node_map, uint32_t cnt)
{
    for (uint32_t blk = 0; blk < cnt; ++blk) {
        struct BlockBuffer *buffer = node_map[blk];
        for (uint32_t byte = 0; byte < PER_BLOCK_BYTES; ++byte) {
            for (uint32_t bit = 0; bit < 8; ++bit) {
                if (_get_bit(&buffer->bf_data[byte], bit) == 0) {
                    _set_bit(&buffer->bf_data[byte], bit);
                    return ((blk * PER_BLOCK_BYTES) << 3) + (byte << 3) + bit;
                }
            }
        }
    }
    return 0;
}

uint32_t
alloc_zone(uint16_t dev)
{
    const struct SuperBlock *super_block = get_super_block(dev);
    const uint32_t zcnt = super_block->sb_zmap_blocks;
    uint32_t znode = _alloc_bit(znode_map, zcnt);
    return znode;
}

uint32_t
get_zone(struct IndexNode *inode, uint32_t bytes_offset, uint32_t *offset_in_blk)
{
    struct PartionEntity *entity = get_partion_entity(inode->in_dev);
    const uint32_t nstart = entity->pe_lba_start / PER_BLOCK_SECTORS;
    // TODO : 这里暂时假设zone和block大小相同
    uint32_t block_num;
    const uint32_t zone_num = bytes_offset >> BLOCK_LOG_SIZE;
    if (offset_in_blk)
        *offset_in_blk = bytes_offset & (PER_BLOCK_BYTES - 1);
    // 直接索引
    if (zone_num < DIRECT_ZONE) {
        block_num = inode->in_inode.in_zones[zone_num];
    }
    // 间接索引
    else if (zone_num < (DIRECT_ZONE + INDIRECT_ZONES)) {
        const uint32_t in_blk = inode->in_inode.in_zones[DIRECT_ZONE];
        const uint32_t in_num = zone_num - DIRECT_ZONE;
        struct BlockBuffer *buf = get_block(inode->in_dev, nstart+in_blk);
        block_num = ((uint32_t *)buf->bf_data)[in_num];
        release_block(buf);
    }
    // 双间接索引
    else if (zone_num < (DIRECT_ZONE + INDIRECT_ZONES + DINDIRECT_ZONES)) {
        uint32_t db_blk = inode->in_inode.in_zones[DIRECT_ZONE + 1];
        uint32_t db_num = zone_num - DIRECT_ZONE - INDIRECT_ZONES;

        uint32_t db_offset = db_num / INDIRECT_ZONES;
        uint32_t db_inoffset = db_num % INDIRECT_ZONES;

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
        uint32_t tr_num = zone_num - DIRECT_ZONE - INDIRECT_ZONES - DINDIRECT_ZONES;

        uint32_t tr_offset = tr_num / DINDIRECT_ZONES;
        uint32_t tr_inoffset = (tr_num % DINDIRECT_ZONES)/INDIRECT_ZONES;
        uint32_t tr_dboffset = tr_num % INDIRECT_ZONES;

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

    return nstart + block_num;
}

static inline uint32_t
_get_znone_begin(uint16_t dev)
{
    struct PartionEntity *entity = get_partion_entity(dev);
    const uint32_t nstart = entity->pe_lba_start / PER_BLOCK_SECTORS;
    const struct SuperBlock *super_block = get_super_block(dev);
    return nstart + super_block->sb_first_datazone;
}
