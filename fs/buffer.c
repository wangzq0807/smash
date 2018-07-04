#include "buffer.h"
#include "hard_disk.h"
#include "memory.h"
#include "log.h"
#include "asm.h"

// configurable
#define BUFFER_LIST_LEN (PAGE_SIZE / sizeof(struct BlockBuffer))
#define BUFFER_HASH_LEN 100

#define HASH_MAGIC   (BUFFER_HASH_LEN * 1000 / 618)
#define HASH(val)    ((val)*HASH_MAGIC % BUFFER_HASH_LEN)

// 空闲双向循环链表
static struct ListHead free_buffers;
static struct BlockBuffer *hash_map[BUFFER_HASH_LEN];

static struct BlockBuffer *buffer_new();
static void wait_for(struct BlockBuffer *buffer);

error_t
init_block_buffer()
{
    struct BlockBuffer *buf = (struct BlockBuffer*)alloc_page();
    // hash_map = (struct BlockBuffer*)alloc_page();

    struct BlockBuffer *iter;

    for (int i = 0; i < BUFFER_LIST_LEN; ++i) {
        iter = &buf[i];
        iter->bf_data = (uint8_t*)(BLK_BUFFER + i*BLK_BUFFER_SIZE);
        iter->bf_refs = 0;
        iter->bf_dev = 0;
        iter->bf_blk = 0;
        iter->bf_status = BUF_FREE;
        push_back(&free_buffers, &iter->bf_link);
    }
    free_buffers.lh_lock = 0;

    return 0;
}

static error_t
_remove_hash_entity(struct BlockBuffer *buf)
{
    uint32_t hash_val = HASH(buf->bf_blk);
    struct BlockBuffer *head = hash_map[hash_val];
    if (head == buf)
        hash_map[hash_val] = buf->bf_hash_next;

    struct BlockBuffer *hash_prev = buf->bf_hash_prev;
    struct BlockBuffer *hash_next = buf->bf_hash_next;
    if (hash_prev != NULL)
        hash_prev->bf_hash_next = hash_next;
    if (hash_next != NULL)
        hash_next->bf_hash_prev = hash_prev;
    buf->bf_hash_prev = NULL;
    buf->bf_hash_next = NULL;

    return 0;
}

static struct BlockBuffer *
_get_hash_entity(dev_t dev, blk_t blk)
{
    uint32_t hash_val = HASH(blk);
    struct BlockBuffer *buf = hash_map[hash_val];
    while (buf != NULL) {
        if (buf->bf_dev == dev &&
            buf->bf_blk == blk)
            return buf;
        buf = buf->bf_hash_next;
    }
    return NULL;
}

static error_t
_put_hash_entity(struct BlockBuffer *buf)
{
    struct BlockBuffer *org = _get_hash_entity(buf->bf_dev, buf->bf_blk);
    if (org != NULL)
        _remove_hash_entity(org);

    uint32_t hash_val = HASH(buf->bf_blk);
    struct BlockBuffer *head = hash_map[hash_val];
    buf->bf_hash_next = head;
    buf->bf_hash_prev = NULL;
    if (head != NULL)
        head->bf_hash_prev = buf;
    hash_map[hash_val] = buf;

    return 0;
}

static struct BlockBuffer *
_get_block(dev_t dev, blk_t blk)
{
    // 参考《unix操作系统设计》的五种场景。
    while (1) {
        struct BlockBuffer *buf = _get_hash_entity(dev, blk);
        if (buf != NULL) {
            if (buf->bf_refs++ == 0)
                remove_entity(&free_buffers, &buf->bf_link);
            if (buf->bf_status & BUF_BUSY) {
                // 5. 缓存命中，但缓冲区状态为"busy"
                // TODO: sleep for buf
                // NOTE: 醒来后无须再次搜索，
                // 引用计数可以保证在进程醒来后，buf没有被释放
            }
            else {
                // 1. 在Hash表中找到了指定block，并且这个block是空闲的
            }
            return buf;
        }
        struct BlockBuffer *new_buffer = buffer_new();
        if (new_buffer == NULL) {
            // 4. free list已经为空
            // TODO: sleep for empty
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

struct BlockBuffer *
get_block(dev_t dev, blk_t blk) 
{
    struct BlockBuffer *buf = _get_block(dev, blk);
    if (buf->bf_status == BUF_BUSY) {
        ata_read(buf);
        wait_for(buf);
    }
    return buf;
}
// error_t put_block(const struct Buffer *buf)
// {
//     lock buffer;
//     block_write(buf->dev, buf->block_num, buf->buffer);
// }

static struct BlockBuffer *
buffer_new( )
{
    if (free_buffers.lh_list == NULL)
        return NULL;

    struct ListEntity *p = pop_front(&free_buffers);
    struct BlockBuffer *ret = TO_INSTANCE(p, BlockBuffer, bf_link);
    ret->bf_refs = 1;
    _remove_hash_entity(ret);

    return ret;
}
// 1. 一个进程不再需要这个缓冲区
// 2. 读写完成时
error_t
release_block(struct BlockBuffer *buf)
{
    if ((buf->bf_refs) && (--buf->bf_refs != 0)) return 0;
    // TODO: 唤醒等待当前缓冲区的进程
    // TODO: 唤醒等待空闲缓冲区的进程
    if (buf->bf_status == BUF_FREE) {
        push_back(&free_buffers, &buf->bf_link);
    }
    else if (buf->bf_status & BUF_BUSY) {
        // 读写尚未完成，就想释放buf?
        // 必须先等待读写完成!!!
        // TODO: 以异步方式等待磁盘
        wait_for(buf);
        push_back(&free_buffers, &buf->bf_link);
    }
    else if (buf->bf_status & BUF_DIRTY) {
        buf->bf_status = BUF_BUSY;
        ata_write(buf);
        // TODO: 以异步方式写磁盘
        wait_for(buf);
        push_back(&free_buffers, &buf->bf_link);
    }

    return 0;
}

static void
wait_for(struct BlockBuffer *buffer)
{
    while (buffer->bf_status != BUF_FREE) {
        // NOTE : 告诉gcc，内存被修改，必须重新从内存中读取bf_status的值
        // 如果不注明的话，判断条件仅会执行一次，从而形成死循环
        asm volatile("":::"memory");
        pause();
    }
}