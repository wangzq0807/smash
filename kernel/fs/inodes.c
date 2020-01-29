#include "inodes.h"
#include "superblk.h"
#include "buffer.h"
#include "memory.h"
#include "fsdefs.h"
#include "lib/log.h"
#include "string.h"
#include "partion.h"
#include "defs.h"

// NOTE : 这里要能整除
#define INODES_OF_ONEBLOCK    (BLOCK_SIZE/sizeof(PyIndexNode))
// 内存中inode最大数量
#define MAX_INODE_NUM   256
// IndexNode的hash表
#define BUFFER_HASH_LEN 64
#define HASH(val)    ((val)%BUFFER_HASH_LEN)

static List free_inodes;
static IndexNode inode_list[MAX_INODE_NUM];

static HashMap  ihash_map;
static HashList ihash_list[BUFFER_HASH_LEN];

int inode_eq(HashNode *node, void* target)
{
    IndexNode* inode = LIST_ENTRY(node, IndexNode, in_hashnode);
    if (inode->in_inum == (size_t)target)
        return 1;
    else
        return 0;
}

BlockBuffer *inode_map[MAX_IMAP_NUM] = {0};

error_t
init_inodes(dev_t dev)
{
    const blk_t superblk_begin = get_super_block_pos(dev);
    const SuperBlock *super_block = get_super_block(dev);
    const int icnt = super_block->sb_imap_blocks;

    const blk_t inode_begin = superblk_begin + SUPER_BLOCK_SIZE;
    for (int i = 0; i < icnt && i < MAX_IMAP_NUM; ++i) {
        inode_map[i] = get_block(dev, inode_begin + i);
    }

    list_init(&free_inodes);
    for (int i = 0; i < MAX_INODE_NUM; ++i) {
        IndexNode *inode = &inode_list[i];
        inode->in_status = INODE_FREE;
        inode->in_refs = 0;
        inode->in_inum = 0;
        list_push_back(&free_inodes, &inode->in_link);
    }

    hash_init(&ihash_map, ihash_list, BUFFER_HASH_LEN, inode_eq);

    return 0;
}

static ino_t
_alloc_bitmap(BlockBuffer **node_map, blk_t cnt)
{
    for (blk_t blk = 0; blk < cnt; ++blk) {
        BlockBuffer *buffer = node_map[blk];
        for (int num = 0; num < BLOCK_SIZE/sizeof(int); ++num) {
            const int bits = sizeof(int) * 8;
            for (int bit = 0; bit < bits; ++bit) {
                if (_get_bit(((int *)buffer->bf_data)[num], bit) == 0) {
                    _set_bit(&((int *)buffer->bf_data)[num], bit);
                    buffer->bf_status |= BUF_DIRTY;
                    return ((blk * BLOCK_SIZE) << 3) + num * bits  + bit;
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
    blk_t index = bits / (sizeof(int)*8);
    int bit = bits % (sizeof(int)*8);
    BlockBuffer *buffer = node_map[num];
    _clear_bit(&((int *)buffer->bf_data)[index], bit);
    buffer->bf_status |= BUF_DIRTY;
    return 0;
}

static inline blk_t
_get_inode_pos(dev_t dev)
{
    const SuperBlock *super_block = get_super_block(dev);
    const blk_t superblk_begin = get_super_block_pos(dev);
    const blk_t superblk_end = superblk_begin + SUPER_BLOCK_SIZE;
    return superblk_end + super_block->sb_imap_blocks + super_block->sb_zmap_blocks;
}

static error_t
_remove_hash_entity(IndexNode *inode)
{
    size_t inum = (size_t)inode->in_inum;
    if (inum == 0)
        return 0;
    hash_t hashid = HASH(inum);
    hash_rm(&ihash_map, hashid, (void*)inum);
    return 0;
}

static IndexNode *
_get_hash_entity(dev_t dev, ino_t idx)
{
    size_t inum = (size_t)idx;
    hash_t hashid = HASH(inum);
    HashNode *node = hash_get(&ihash_map, hashid, (void*)inum);
    if (node != NULL)
        return LIST_ENTRY(node, IndexNode, in_hashnode);
    return NULL;
}

static error_t
_put_hash_entity(IndexNode *inode)
{
    size_t inum = (size_t)inode->in_inum;
    inode->in_hashnode.hn_key = HASH(inum);
    hash_put(&ihash_map, &inode->in_hashnode, (void*)inum);
    return 0;
}

IndexNode *
alloc_inode(dev_t dev)
{
    KLOG(DEBUG, "alloc_inode");
    // 申请一个新的IndexNode
    IndexNode *inode = NULL;
    if (list_size(&free_inodes) == 0) {
        // TODO : error
    }
    else {
        ListNode *p = list_pop_front(&free_inodes);
        if (p == NULL)
            KLOG(ERROR, "buffer_new, no free buffer!");
        inode = LIST_ENTRY(p, IndexNode, in_link);
        _remove_hash_entity(inode);
    }
    // 为新inode分配一个bit位
    const SuperBlock *super_block = get_super_block(dev);
    const blk_t icnt = super_block->sb_imap_blocks;
    const ino_t inode_index = _alloc_bitmap(inode_map, icnt);
    // 初始化
    inode->in_dev = dev;
    inode->in_status = INODE_DIRTY;
    inode->in_inum = inode_index;
    inode->in_refs = 1;
    _put_hash_entity(inode);
    return inode;
}

error_t
delete_inode(IndexNode *inode)
{
    _clear_bitmap(inode_map, inode->in_inum);
    release_inode(inode);
    return 0;
}

IndexNode *
get_inode(dev_t dev, ino_t inode_index)
{
    KLOG(DEBUG, "get_inode %d", inode_index);
    hash_for_each(&ihash_map, iter) {
        KLOG(DEBUG, "%d", iter->hn_key);
    }
    KLOG(DEBUG, "get_inode end");
    IndexNode *inode = NULL;
    while (inode == NULL) {
        inode = _get_hash_entity(dev, inode_index);
        if (inode != NULL) {
            if (inode->in_refs == 0)
                list_remove_entity(&free_inodes, &inode->in_link);
            inode->in_refs += 1;
            if (inode->in_status == INODE_LOCK) {
                inode = NULL;
                // TODO : sleep for inode
                // continue;
            }
            return inode;
        }
        else {
            // 申请一个新的IndexNode
            if (list_size(&free_inodes) == 0) {
                // TODO : error
                KLOG(ERROR, "no free inodes!!!");
            }
            else {
                ListNode *p = list_pop_front(&free_inodes);
                if (p == NULL)
                    KLOG(ERROR, "buffer_new, no free buffer!");
                inode = LIST_ENTRY(p, IndexNode, in_link);
                _remove_hash_entity(inode);
            }
            // 读磁盘上的inode
            const blk_t inode_begin = _get_inode_pos(dev);
            const blk_t block_num = (inode_index - 1) / INODES_OF_ONEBLOCK + inode_begin;
            const ino_t offset = (inode_index - 1) % INODES_OF_ONEBLOCK;

            BlockBuffer *buffer = get_block(dev, block_num);
            // 将inode的内容拷贝到IndexNode
            uint8_t *ptr = buffer->bf_data + offset * sizeof(PyIndexNode);
            memcpy(&inode->in_inode, ptr, sizeof(PyIndexNode));
            release_block(buffer);
            // 初始化
            inode->in_dev = dev;
            inode->in_status = INODE_FREE;
            inode->in_inum = inode_index;
            inode->in_refs = 1;
            _put_hash_entity(inode);
        }
    }
    return (IndexNode *)inode;
}

void
release_inode(IndexNode *inode)
{
    inode->in_refs -= 1;
    if (inode->in_refs == 0) {
        if (inode->in_status & INODE_DIRTY) {
            // 读磁盘上的inode缓冲区
            const blk_t inode_begin = _get_inode_pos(inode->in_dev);
            const blk_t block_num = (inode->in_inum - 1) / INODES_OF_ONEBLOCK + inode_begin;
            const ino_t offset = (inode->in_inum - 1) % INODES_OF_ONEBLOCK;

            BlockBuffer *buffer = get_block(inode->in_dev, block_num);
            // 将IndexNode的内容拷贝到inode缓冲区
            uint8_t *ptr = buffer->bf_data + offset * sizeof(PyIndexNode);
            memcpy(ptr, &inode->in_inode, sizeof(PyIndexNode));
            buffer->bf_status = BUF_DIRTY;
            release_block(buffer);
        }
        inode->in_status = INODE_FREE;
        // 将inode放入空闲列表(NOTE:仍然存在于hash表中)
        list_push_back(&free_inodes, &inode->in_link);
    }
    inode->in_status = 0;
}

void
sync_inodes(dev_t dev)
{
    const SuperBlock *super_block = get_super_block(dev);
    const int icnt = super_block->sb_imap_blocks;

    for (int i = 0; i < MIN(icnt, MAX_IMAP_NUM); ++i) {
        if (inode_map[i]->bf_status & BUF_DIRTY) {
            dev = inode_map[i]->bf_dev;
            blk_t blk = inode_map[i]->bf_blk;
            release_block(inode_map[i]);
            inode_map[i] = get_block(dev, blk);
        }
    }
}
