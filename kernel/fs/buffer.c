#include "buffer.h"
#include "hard_disk.h"
#include "memory.h"
#include "lib/log.h"
#include "asm.h"

// configurable
#define BUFFER_LIST_LEN (PAGE_SIZE / sizeof(BlockBuffer))
#define BUFFER_HASH_LEN 128

#define HASH(val)    ((val) % BUFFER_HASH_LEN)

// 空闲双向循环链表
static List free_buffers;
// Hash 表
static HashMap  bhash_map;
static HashList bhash_list[BUFFER_HASH_LEN];
static int bf_eq(HashNode* node, void* blk) {
    BlockBuffer* bf = LIST_ENTRY(node, BlockBuffer, bf_hashnode);
    if (bf->bf_blk == (hash_t)blk)
        return 1;
    else
        return 0;
}

static BlockBuffer *buffer_new();
static void wait_for(BlockBuffer *buffer);

error_t
init_block_buffer()
{
    BlockBuffer *buf = (BlockBuffer*)vm_alloc();

    BlockBuffer *iter;

    for (int i = 0; i < BUFFER_LIST_LEN; ++i) {
        iter = &buf[i];
        iter->bf_data = (uint8_t*)vm_alloc();
        iter->bf_refs = 0;
        iter->bf_dev = 0;
        iter->bf_blk = 0;
        iter->bf_status = BUF_FREE;
        list_push_back(&free_buffers, &iter->bf_link);
    }
    hash_init(&bhash_map, bhash_list, BUFFER_HASH_LEN, bf_eq);

    return 0;
}

static error_t
_remove_hash_entity(BlockBuffer *buf)
{
    uint32_t hash_key = HASH(buf->bf_blk);
    hash_rm(&bhash_map, hash_key, (size_t*)buf->bf_blk);
    return 0;
}

static BlockBuffer *
_get_hash_entity(dev_t dev, blk_t blk)
{
    uint32_t hash_key = HASH(blk);
    HashNode* pNode = hash_get(&bhash_map, hash_key, (size_t*)blk);
    if (pNode != NULL)
        return LIST_ENTRY(pNode, BlockBuffer, bf_hashnode);
    return NULL;
}

static error_t
_put_hash_entity(BlockBuffer *buf)
{
    hash_put(&bhash_map, &buf->bf_hashnode, (size_t*)buf->bf_blk);

    return 0;
}

static BlockBuffer *
_get_block(dev_t dev, blk_t blk)
{
    // 参考《unix操作系统设计》的五种场景。
    while (1) {
        BlockBuffer *buf = _get_hash_entity(dev, blk);
        if (buf != NULL) {
            if (buf->bf_refs++ == 0)
                list_remove_entity(&free_buffers, &buf->bf_link);
            if (buf->bf_status & BUF_BUSY) {
                // 5. 缓存命中，但缓冲区状态为"busy"
                // TODO: sleep for buf
                // NOTE: 醒来后无须再次搜索，
                // 引用计数可以保证在进程醒来后，buf没有被释放
                wait_for(buf);
                buf->bf_status = BUF_FREE;
            }
            else {
                // 1. 在Hash表中找到了指定block，并且这个block是空闲的
            }
            return buf;
        }
        BlockBuffer *new_buffer = buffer_new();
        if (new_buffer == NULL) {
            // 4. free list已经为空
            // TODO: sleep for empty
            KLOG(ERROR, " empty ");
            while (1);
            continue;
        }
        else if (new_buffer->bf_status & BUF_DELAYWRITE) {
            // 3. 新申请的缓冲区的状态是"delay write"，因此需要先写入，然后申请另一块
            // TODO : 这个状态太复杂，暂不实现
            // TODO : 写磁盘，写完成后重新放入队列头部
            // NOTE : 不能由中断响应函数来释放这个缓冲区
            continue;
        }
        else {
            // 2. Hash表中没有找到指定block，新申请一块空的缓冲区
            new_buffer->bf_dev = dev;
            new_buffer->bf_blk = blk;
            new_buffer->bf_status = BUF_BUSY;
            _put_hash_entity(new_buffer);
            return new_buffer;
        }
    }

    return NULL;
}

BlockBuffer *
get_block(dev_t dev, blk_t blk) 
{
    BlockBuffer *buf = _get_block(dev, blk);
    if (buf->bf_status == BUF_BUSY) {
        ata_read(buf);
        wait_for(buf);
    }
    return buf;
}
// error_t put_block(const Buffer *buf)
// {
//     lock buffer;
//     block_write(buf->dev, buf->block_num, buf->buffer);
// }

static BlockBuffer *
buffer_new()
{
    if (list_size(&free_buffers) == 0)
        return NULL;

    ListNode *p = list_pop_front(&free_buffers);
    BlockBuffer *ret = LIST_ENTRY(p, BlockBuffer, bf_link);
    ret->bf_refs = 1;
    _remove_hash_entity(ret);

    return ret;
}
// 1. 一个进程不再需要这个缓冲区
// 2. 读写完成时
error_t
release_block(BlockBuffer *buf)
{
    if ((buf->bf_refs) && (--buf->bf_refs != 0)) return 0;
    // TODO: 唤醒等待当前缓冲区的进程
    // TODO: 唤醒等待空闲缓冲区的进程
    if (buf->bf_status == BUF_FREE) {
        list_push_back(&free_buffers, &buf->bf_link);
    }
    else if (buf->bf_status & BUF_BUSY) {
        // 读写尚未完成，就想释放buf?
        // 必须先等待读写完成!!!
        // TODO: 以异步方式等待磁盘
        wait_for(buf);
        list_push_back(&free_buffers, &buf->bf_link);
    }
    else if (buf->bf_status & BUF_DIRTY) {
        buf->bf_status = BUF_BUSY;
        ata_write(buf);
        // TODO: 以异步方式写磁盘
        wait_for(buf);
        list_push_back(&free_buffers, &buf->bf_link);
    }

    return 0;
}

void sync_dev(dev_t dev)
{
    hash_for_each(&bhash_map, iter) {
        BlockBuffer *buf = LIST_ENTRY(iter, BlockBuffer, bf_hashnode);
        if (buf->bf_status & BUF_DIRTY) {
            buf->bf_status = BUF_BUSY;
            ata_write(buf);
            // TODO: 以异步方式写磁盘
            wait_for(buf);
        }
    }
}

static void
wait_for(BlockBuffer *buffer)
{
    while (buffer->bf_status != BUF_FREE) {
        // NOTE : 告诉gcc，内存被修改，必须重新从内存中读取bf_status的值
        // 如果不注明的话，判断条件仅会执行一次，从而形成死循环
        __asm__ volatile("":::"memory");
        __asm__ volatile ("pause");
    }
}
