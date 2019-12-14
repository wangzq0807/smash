#include "nodes.h"
#include "disk_drv.h"
#include "superblk.h"
#include "memory.h"
#include "log.h"
#include "string.h"

// 每个块的扇区数
#define SECTORS_OF_ONEBLOCK   (BLOCK_SIZE/SECTOR_SIZE)
// 每个block上Index Node的数量
#define INODES_OF_ONEBLOCK    (BLOCK_SIZE/sizeof(IndexNode))

uint8_t bitmap_buf[BLOCK_SIZE]; // 位图缓存
int bitmap_bufidx = -1;         // 位图缓存的索引

IndexNode   tmpinode;

void*
get_inodes_bitmap(ino_t inode_index)
{
    // 如果inode位图已在内存中,则直接返回
    const int bitmap_index = (inode_index >> BLOCK_SHIFT) / 8;
    if (bitmap_bufidx == bitmap_index)
        return bitmap_buf;
    // 从磁盘中去读inode位图
    const blk_t superblk_begin = get_super_block_pos();
    const SuperBlock *super_block = get_super_block();
    const int icnt = super_block->sb_imap_blocks;
    if (bitmap_index >= icnt)
        return NULL;
    // 获取inode位图在磁盘上的开始位置
    const blk_t inodemap_begin = superblk_begin + SUPER_BLOCK_SIZE;
    // 读取inode位图
    void *blockbuf = ata_read(inodemap_begin + bitmap_index);
    memcpy(bitmap_buf, blockbuf, BLOCK_SIZE);
    bitmap_bufidx = bitmap_index;

    return bitmap_buf;
}

static inline blk_t
_get_inode_pos()
{
    const SuperBlock *super_block = get_super_block();
    const blk_t superblk_begin = get_super_block_pos();
    const blk_t superblk_end = superblk_begin + SUPER_BLOCK_SIZE;
    return superblk_end + super_block->sb_imap_blocks + super_block->sb_zmap_blocks;
}

IndexNode*
get_inode(ino_t inode_index)
{
    KLOG(DEBUG, "get_inode: %d", inode_index);
    // 检查请求的inode是否存在.
    const uint8_t* bitmap = (uint8_t*)get_inodes_bitmap(inode_index);
    const int bytenum = (inode_index >> 3) & (BLOCK_SIZE - 1);
    const uint8_t onebyte = bitmap[bytenum];
    int bit = _get_bit(onebyte, inode_index & 7);
    if (bit == 0) {
        KLOG(ERROR, "get_inode : %d bit is not set", inode_index);
        return NULL;
    }

    // 读磁盘上的inode
    const blk_t inode_begin = _get_inode_pos();
    const blk_t block_num = (inode_index - 1) / INODES_OF_ONEBLOCK + inode_begin;
    const ino_t offset = (inode_index - 1) % INODES_OF_ONEBLOCK;

    void *blockbuf = ata_read(block_num);
    // 将inode的内容拷贝到IndexNode
    uint8_t *ptr = blockbuf + offset * sizeof(IndexNode);
    memcpy(&tmpinode, ptr, sizeof(IndexNode));

    KLOG(DEBUG, "get_inode filesize: %d", tmpinode.in_file_size);

    return &tmpinode;
}

zone_t
get_zone(const IndexNode *inode, off_t bytes_offset)
{
    PartionEntity *entity = get_partion_entity();
    const blk_t nstart = entity->pe_lba_start / SECTORS_OF_ONEBLOCK;
    // TODO : 这里暂时假设zone和block大小相同
    blk_t block_num = 0;
    const zone_t zone_num = bytes_offset >> BLOCK_SHIFT;
    // 直接索引
    if (zone_num < DIRECT_ZONE) {
        block_num = inode->in_zones[zone_num];
    }
    // 间接索引
    else if (zone_num < (DIRECT_ZONE + INDIRECT_ZONES)) {
        const uint32_t in_blk = inode->in_zones[DIRECT_ZONE];
        const zone_t in_total_num = zone_num - DIRECT_ZONE;
        void *blockbuf = ata_read(nstart+in_blk);
        block_num = ((uint32_t *)blockbuf)[in_total_num];
    }
    // 双间接索引
    else if (zone_num < (DIRECT_ZONE + INDIRECT_ZONES + DINDIRECT_ZONES)) {
        uint32_t db_blk = inode->in_zones[DIRECT_ZONE + 1];
        zone_t db_total_num = zone_num - DIRECT_ZONE - INDIRECT_ZONES;

        zone_t db_offset = db_total_num / INDIRECT_ZONES;
        zone_t db_inoffset = db_total_num % INDIRECT_ZONES;

        void *blockbuf = ata_read(nstart+db_blk);
        uint32_t inblock_num = ((uint32_t *)blockbuf)[db_offset];

        blockbuf = ata_read(nstart+inblock_num);
        block_num = ((uint32_t *)blockbuf)[db_inoffset];
    }
    // 三间接索引
    else if (zone_num < (DIRECT_ZONE + INDIRECT_ZONES + DINDIRECT_ZONES + TINDIRECT_ZONES)) {
        uint32_t tr_blk = inode->in_zones[DIRECT_ZONE + 2];
        zone_t tr_total_num = zone_num - DIRECT_ZONE - INDIRECT_ZONES - DINDIRECT_ZONES;

        zone_t tr_offset = tr_total_num / DINDIRECT_ZONES;
        zone_t tr_inoffset = (tr_total_num % DINDIRECT_ZONES)/INDIRECT_ZONES;
        zone_t tr_dboffset = tr_total_num % INDIRECT_ZONES;

        void *blockbuf = ata_read(nstart+tr_blk);
        uint32_t inblock_num = ((uint32_t *)blockbuf)[tr_offset];

        blockbuf = ata_read(nstart+inblock_num);
        uint32_t dbblock_num = ((uint32_t *)blockbuf)[tr_inoffset];

        blockbuf = ata_read(nstart+dbblock_num);
        block_num = ((uint32_t *)blockbuf)[tr_dboffset];
    }
    if (block_num != 0)
        return nstart + block_num;
    else
        return 0;
}


