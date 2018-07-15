#include "zones.h"
#include "inodes.h"
#include "buffer.h"
#include "superblk.h"
#include "partion.h"

#define PER_BLOCK_SECTORS   (BLOCK_SIZE/SECTOR_SIZE)
// 间接索引块数
#define INDIRECT_ZONES      (BLOCK_SIZE / sizeof(zone_t))
// 双间接索引
#define DINDIRECT_ZONES     (INDIRECT_ZONES * INDIRECT_ZONES)
// 三间接索引
#define TINDIRECT_ZONES     (INDIRECT_ZONES * INDIRECT_ZONES * INDIRECT_ZONES)

// TODO: znode_map数组的大小是不够用的
BlockBuffer *znode_map[MAX_ZMAP_NUM] = {0};

error_t
init_zones(dev_t dev)
{
    const blk_t superblk_begin = get_super_block_begin(dev);
    const SuperBlock *super_block = get_super_block(dev);
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
_alloc_bitmap(BlockBuffer **node_map, blk_t cnt)
{
    for (blk_t blk = 0; blk < cnt; ++blk) {
        BlockBuffer *buffer = node_map[blk];
        for (int num = 0; num < BLOCK_SIZE/sizeof(int); ++num) {
            int bits = sizeof(int) * 8;
            for (int bit = 0; bit < bits; ++bit) {
                if (_get_bit(((int *)buffer->bf_data)[num], bit) == 0) {
                    _set_bit(&((int *)buffer->bf_data)[num], bit);
                    buffer->bf_status |= BUF_DIRTY;
                    return ((blk * BLOCK_SIZE) << 3) + num * bits + bit;
                }
            }
        }
    }
    return 0;
}

static error_t
_clear_bitmap(BlockBuffer **node_map, blk_t cnt)
{
    int blkbits = BLOCK_SIZE << 3;
    blk_t num = cnt / blkbits;
    blk_t bits = cnt % blkbits;
    blk_t index = bits / sizeof(int);
    int bit = bits % sizeof(int);
    BlockBuffer *buffer = node_map[num];
    _clear_bit(&((int *)buffer->bf_data)[index], bit);
    return 0;
}

zone_t
alloc_zone(IndexNode *inode)
{
    dev_t dev = inode->in_dev;
    const SuperBlock *super_block = get_super_block(dev);
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
get_zone(IndexNode *inode, seek_t bytes_offset)
{
    PartionEntity *entity = get_partion_entity(inode->in_dev);
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
        BlockBuffer *buf = get_block(inode->in_dev, nstart+in_blk);
        block_num = ((uint32_t *)buf->bf_data)[in_total_num];
        release_block(buf);
    }
    // 双间接索引
    else if (zone_num < (DIRECT_ZONE + INDIRECT_ZONES + DINDIRECT_ZONES)) {
        uint32_t db_blk = inode->in_inode.in_zones[DIRECT_ZONE + 1];
        zone_t db_total_num = zone_num - DIRECT_ZONE - INDIRECT_ZONES;

        zone_t db_offset = db_total_num / INDIRECT_ZONES;
        zone_t db_inoffset = db_total_num % INDIRECT_ZONES;

        BlockBuffer *buf = get_block(inode->in_dev, nstart+db_blk);
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

        BlockBuffer *buf = get_block(inode->in_dev, nstart+tr_blk);
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

static error_t
_truncate_direct_zones(IndexNode *inode, blk_t nstart)
{
    for (blk_t num = 0; num < DIRECT_ZONE; ++num) {
        blk_t zone = inode->in_inode.in_zones[num];
        if(zone == INVALID_ZONE)
            return 0;
        _clear_bitmap(znode_map, zone);
        inode->in_inode.in_zones[num] = INVALID_ZONE;
    }
    return 0;
}

static error_t
_truncate_indirect_zones(IndexNode *inode, blk_t nstart)
{
    blk_t inblk = inode->in_inode.in_zones[DIRECT_ZONE];
    if (inblk == INVALID_ZONE)
        return 0;
    BlockBuffer *buf = get_block(inode->in_dev, nstart+inblk);
    for (blk_t innum = 0; innum < BLOCK_SIZE / sizeof(blk_t); ++innum) {
        blk_t zone = ((blk_t *)buf->bf_data)[innum];
        if(zone == INVALID_ZONE)
            return 0;
        _clear_bitmap(znode_map, zone);
    }
    release_block(buf);
    _clear_bitmap(znode_map, inblk);
    inode->in_inode.in_zones[DIRECT_ZONE] = INVALID_ZONE;
    return 0;
}

static error_t
_truncate_dbdirect_zones(IndexNode *inode, blk_t nstart)
{
    blk_t dbblk = inode->in_inode.in_zones[DIRECT_ZONE+1];
    if (dbblk == INVALID_ZONE)
        return 0;
    BlockBuffer *dbbuf = get_block(inode->in_dev, nstart+dbblk);
    for (blk_t dbnum = 0; dbnum < BLOCK_SIZE / sizeof(blk_t); ++dbnum) {
        blk_t inblk = ((blk_t *)dbbuf->bf_data)[dbnum];
        if (inblk == INVALID_ZONE)
            return 0;
        BlockBuffer *buf = get_block(inode->in_dev, nstart+inblk);
        for (blk_t innum = 0; innum < BLOCK_SIZE / sizeof(blk_t); ++innum) {
            blk_t zone = ((blk_t *)buf->bf_data)[innum];
            if(zone == INVALID_ZONE)
                return 0;
            _clear_bitmap(znode_map, zone);
        }
        release_block(buf);
        _clear_bitmap(znode_map, inblk);
    }
    release_block(dbbuf);
    _clear_bitmap(znode_map, dbblk);
    inode->in_inode.in_zones[DIRECT_ZONE+1] = INVALID_ZONE;
    return 0;
}

static error_t
_truncate_trdirect_zones(IndexNode *inode, blk_t nstart)
{
    blk_t trblk = inode->in_inode.in_zones[DIRECT_ZONE+2];
    if (trblk == INVALID_ZONE)
        return 0;
    BlockBuffer *trbuf = get_block(inode->in_dev, nstart+trblk);
    for (blk_t trnum = 0; trnum < BLOCK_SIZE / sizeof(blk_t); ++trnum) {
        blk_t dbblk = ((blk_t *)trbuf->bf_data)[trnum];
        if (dbblk == INVALID_ZONE)
            return 0;
        BlockBuffer *dbbuf = get_block(inode->in_dev, nstart+dbblk);
        for (blk_t dbnum = 0; dbnum < BLOCK_SIZE / sizeof(blk_t); ++dbnum) {
            blk_t inblk = ((blk_t *)dbbuf->bf_data)[dbnum];
            if (inblk == INVALID_ZONE)
                return 0;
            BlockBuffer *buf = get_block(inode->in_dev, nstart+inblk);
            for (blk_t innum = 0; innum < BLOCK_SIZE / sizeof(blk_t); ++innum) {
                blk_t zone = ((blk_t *)buf->bf_data)[innum];
                if(zone == INVALID_ZONE)
                    return 0;
                _clear_bitmap(znode_map, zone);
            }
            release_block(buf);
            _clear_bitmap(znode_map, inblk);
        }
        release_block(dbbuf);
        _clear_bitmap(znode_map, dbblk);
    }
    release_block(trbuf);
    _clear_bitmap(znode_map, trblk);
    inode->in_inode.in_zones[DIRECT_ZONE+2] = INVALID_ZONE;
    return 0;
}

error_t
truncate_zones(IndexNode *inode)
{
    PartionEntity *entity = get_partion_entity(inode->in_dev);
    const blk_t nstart = entity->pe_lba_start / PER_BLOCK_SECTORS;
    // 直接索引
    _truncate_direct_zones(inode, nstart);
    // 间接索引
    _truncate_indirect_zones(inode, nstart);
    // 双间接索引
    _truncate_dbdirect_zones(inode, nstart);
    // 三间接索引
    _truncate_trdirect_zones(inode, nstart);
    inode->in_inode.in_file_size = 0;

    return 0;
}

static inline uint32_t
_get_znone_begin(dev_t dev)
{
    PartionEntity *entity = get_partion_entity(dev);
    const uint32_t nstart = entity->pe_lba_start / PER_BLOCK_SECTORS;
    const SuperBlock *super_block = get_super_block(dev);
    return nstart + super_block->sb_first_datazone;
}
