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

struct BlockBufferHead {
    uint32_t            bf_lock;
    struct BlockBuffer  *bf_buf;
};
// 空闲双向循环链表
static struct BlockBufferHead free_buffers;
static struct BlockBuffer *hash_map[BUFFER_HASH_LEN];

static struct BlockBuffer *buffer_new();
static error_t _put_to_freelist(struct BlockBuffer *buf);
static error_t _remove_from_freelist(struct BlockBuffer *buf);
static void wait_for(struct BlockBuffer *buffer);

error_t
init_block_buffer()
{
    struct BlockBuffer *buf = (struct BlockBuffer*)alloc_page();
    // hash_map = (struct BlockBuffer*)alloc_page();

    struct BlockBuffer *iter;
    struct BlockBuffer *prev = &buf[0];
    prev->bf_data = (uint8_t*)BLK_BUFFER;
    prev->bf_refs = 0;

    for (int i = 1; i < BUFFER_LIST_LEN; ++i) {
        iter = &buf[i];
        iter->bf_data = (uint8_t*)(BLK_BUFFER + i*BLK_BUFFER_SIZE);
        iter->bf_refs = 0;
        iter->bf_prev = prev;
        prev->bf_next = iter;
        prev = iter;
    }
    // 连接链表的头和尾
    iter->bf_next = &buf[0];
    buf[0].bf_prev = iter;
    free_buffers.bf_buf = buf;
    free_buffers.bf_lock = 0;

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
                _remove_from_freelist(buf);
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
            _remove_hash_entity(new_buffer);
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
    if (free_buffers.bf_buf == NULL)
        return NULL;

    struct BlockBuffer *ret = free_buffers.bf_buf;
    ret->bf_refs = 1;
    free_buffers.bf_buf = ret->bf_next;

    if (free_buffers.bf_buf == ret) {
        // 最后一个空缓冲被分配完
        free_buffers.bf_buf = NULL;
    }
    else {
        if (ret->bf_prev != NULL)
            ret->bf_prev->bf_next = ret->bf_next;
        if (ret->bf_next != NULL)
            ret->bf_next->bf_prev = ret->bf_prev;
    }

    ret->bf_prev = NULL;
    ret->bf_next = NULL;

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
        _put_to_freelist(buf);
    }
    else if (buf->bf_status & BUF_BUSY) {
        // 读写尚未完成，就想释放buf?
        // 必须先等待读写完成!!!
        // TODO: 以异步方式等待磁盘
        wait_for(buf);
        _put_to_freelist(buf);
    }
    else if (buf->bf_status & BUF_DIRTY) {
        buf->bf_status = BUF_BUSY;
        ata_write(buf);
        // TODO: 以异步方式写磁盘
        wait_for(buf);
        _put_to_freelist(buf);
    }

    return 0;
}

static error_t
_put_to_freelist(struct BlockBuffer *buf) {
    if (free_buffers.bf_buf != NULL) {
        // 放到队列尾部
        buf->bf_next = free_buffers.bf_buf;
        buf->bf_prev = free_buffers.bf_buf->bf_prev;
        buf->bf_prev->bf_next = buf;
        free_buffers.bf_buf->bf_prev = buf;
    }
    else {
        free_buffers.bf_buf = buf;
        buf->bf_next = buf;
        buf->bf_prev = buf;
    }
    return 0;
}

static error_t
_remove_from_freelist(struct BlockBuffer *buf) {
    if (free_buffers.bf_buf == NULL)
        return 0;
    if (buf->bf_prev == NULL || buf->bf_next == NULL)
        return 0;
    struct BlockBuffer* next = buf->bf_next;
    struct BlockBuffer* prev = buf->bf_prev;
    if (next != buf && prev != buf) {
        next->bf_prev = prev;
        prev->bf_next = next;
        if (buf == free_buffers.bf_buf)
            free_buffers.bf_buf = next;
    }
    else {
        free_buffers.bf_buf = NULL;
    }
    buf->bf_next = NULL;
    buf->bf_prev = NULL;

    return -1;
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