#include "inodes.h"
#include "superblk.h"
#include "buffer.h"
#include "memory.h"
#include "fs.h"
#include "log.h"
#include "string.h"
#include "partion.h"

#define PER_BLOCK_BYTES     (1 << BLOCK_LOG_SIZE)
// NOTE : 这里要能整除
#define PER_BLOCK_INODES    (PER_BLOCK_BYTES/sizeof(struct PyIndexNode))
// 内存中inode最大数量
#define MAX_INODE_NUM   1024
// IndexNode的hash表
#define BUFFER_HASH_LEN 100
#define HASH_MAGIC   (BUFFER_HASH_LEN * 1000 / 618)
#define HASH(val)    ((val)*HASH_MAGIC % BUFFER_HASH_LEN)

static struct ListHead free_inodes;
static struct IndexNode *ihash_map[BUFFER_HASH_LEN];

struct BlockBuffer *inode_map[MAX_IMAP_NUM] = {0};

error_t
init_inodes(dev_t dev)
{
    const blk_t superblk_begin = get_super_block_begin(dev);
    const struct SuperBlock *super_block = get_super_block(dev);
    const int icnt = super_block->sb_imap_blocks;

    const blk_t inode_begin = superblk_begin + SUPER_BLOCK_SIZE;
    for (int i = 0; i < icnt && i < MAX_IMAP_NUM; ++i) {
        inode_map[i] = get_block(dev, inode_begin + i);
    }

    free_inodes.lh_list = NULL;
    free_inodes.lh_lock = 0;
    return 0;
}

static ino_t
_alloc_bit(struct BlockBuffer **node_map, blk_t cnt)
{
    for (blk_t blk = 0; blk < cnt; ++blk) {
        struct BlockBuffer *buffer = node_map[blk];
        for (int num = 0; num < PER_BLOCK_BYTES/sizeof(int); ++num) {
            const int bits = sizeof(int) * 8;
            for (int bit = 0; bit < bits; ++bit) {
                if (_get_bit(((int *)buffer->bf_data)[num], bit) == 0) {
                    _set_bit(&((int *)buffer->bf_data)[num], bit);
                    buffer->bf_status |= BUF_DIRTY;
                    return ((blk * PER_BLOCK_BYTES) << 3) + num * bits  + bit;
                }
            }
        }
    }
    return 0;
}

static inline blk_t
_get_inode_begin(dev_t dev)
{
    const struct SuperBlock *super_block = get_super_block(dev);
    const blk_t superblk_begin = get_super_block_begin(dev);
    const blk_t superblk_end = superblk_begin + SUPER_BLOCK_SIZE;
    return superblk_end + super_block->sb_imap_blocks + super_block->sb_zmap_blocks;
}

static error_t
_remove_hash_entity(struct IndexNode *inode)
{
    int hash_val = HASH(inode->in_inum);
    struct IndexNode *head = ihash_map[hash_val];
    if (head == inode)
        ihash_map[hash_val] = inode->in_hash_next;

    struct IndexNode *hash_prev = inode->in_hash_prev;
    struct IndexNode *hash_next = inode->in_hash_next;
    if (hash_prev != NULL)
        hash_prev->in_hash_next = hash_next;
    if (hash_next != NULL)
        hash_next->in_hash_prev = hash_prev;
    inode->in_hash_prev = NULL;
    inode->in_hash_next = NULL;

    return 0;
}

static struct IndexNode *
_get_hash_entity(dev_t dev, ino_t idx)
{
    int hash_val = HASH(idx);
    struct IndexNode *inode = ihash_map[hash_val];
    while (inode != NULL) {
        if (inode->in_dev == dev &&
            inode->in_inum == idx)
            return inode;
        inode = inode->in_hash_next;
    }
    return NULL;
}

static error_t
_put_hash_entity(struct IndexNode *inode)
{
    struct IndexNode *org = _get_hash_entity(inode->in_dev, inode->in_inum);
    if (org != NULL)
        _remove_hash_entity(org);

    int hash_val = HASH(inode->in_inum);
    struct IndexNode *head = ihash_map[hash_val];
    inode->in_hash_next = head;
    inode->in_hash_prev = NULL;
    if (head != NULL)
        head->in_hash_prev = inode;
    ihash_map[hash_val] = inode;

    return 0;
}

struct IndexNode *
alloc_inode(dev_t dev)
{
    // 申请一个新的IndexNode
    struct IndexNode *inode = NULL;
    if (free_inodes.lh_list == NULL)
        inode = (struct IndexNode *)alloc_object(EIndexNode, sizeof(struct IndexNode));
    else {
        struct ListEntity *p = pop_front(&free_inodes);
        inode = TO_INSTANCE(p, IndexNode, in_link);
        _remove_hash_entity(inode);
    }
    // 为新inode分配一个bit位
    const struct SuperBlock *super_block = get_super_block(dev);
    const blk_t icnt = super_block->sb_imap_blocks;
    const ino_t inode_index = _alloc_bit(inode_map, icnt);
    // 初始化
    inode->in_dev = dev;
    inode->in_status = INODE_LOCK;
    inode->in_inum = inode_index;
    inode->in_refs = 1;
    _put_hash_entity(inode);
    return inode;
}

error_t
free_inode(struct IndexNode *inode)
{

    return 0;
}


struct IndexNode *
get_inode(dev_t dev, ino_t inode_index)
{
    struct IndexNode *inode = NULL;
    while (inode == NULL) {
        inode = _get_hash_entity(dev, inode_index);
        if (inode != NULL) {
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
            if (free_inodes.lh_list == NULL)
                inode = (struct IndexNode *)alloc_object(EIndexNode, sizeof(struct IndexNode));
            else {
                struct ListEntity *p = pop_front(&free_inodes);
                inode = TO_INSTANCE(p, IndexNode, in_link);
                _remove_hash_entity(inode);
            }
            // 读磁盘上的inode
            const blk_t inode_begin = _get_inode_begin(dev);
            const blk_t block_num = (inode_index - 1) / PER_BLOCK_INODES + inode_begin;
            const ino_t offset = (inode_index - 1) % PER_BLOCK_INODES;

            struct BlockBuffer *buffer = get_block(dev, block_num);
            // 将inode的内容拷贝到IndexNode
            uint8_t *ptr = buffer->bf_data + offset * sizeof(struct PyIndexNode);
            memcpy(&inode->in_inode, ptr, sizeof(struct PyIndexNode));
            release_block(buffer);
            // 初始化
            inode->in_dev = dev;
            inode->in_status = INODE_LOCK;
            inode->in_inum = inode_index;
            inode->in_refs = 1;
            _put_hash_entity(inode);
        }
    }
    return (struct IndexNode *)inode;
}

void
release_inode(struct IndexNode *inode)
{
    inode->in_refs -= 1;
    if (inode->in_refs == 0) {
        // 将inode放入空闲列表
        push_back(&free_inodes, &inode->in_link);
        // 读磁盘上的inode缓冲区
        const blk_t inode_begin = _get_inode_begin(inode->in_dev);
        const blk_t block_num = (inode->in_inum - 1) / PER_BLOCK_INODES + inode_begin;
        const ino_t offset = (inode->in_inum - 1) % PER_BLOCK_INODES;

        struct BlockBuffer *buffer = get_block(inode->in_dev, block_num);
        // 将IndexNode的内容拷贝到inode缓冲区
        uint8_t *ptr = buffer->bf_data + offset * sizeof(struct PyIndexNode);
        memcpy(ptr, &inode->in_inode, sizeof(struct PyIndexNode));
        release_block(buffer);
    }
    inode->in_status = 0;
}

