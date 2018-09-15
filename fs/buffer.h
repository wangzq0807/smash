#ifndef __BUFFER_H__
#define __BUFFER_H__
#include "sys/types.h"
#include "list.h"

// 缓冲区状态
#define BUF_FREE            0       // 空缓冲区，可以被使用
#define BUF_BUSY            1       // 正在读/写磁盘，不允许被任何进程读或写
#define BUF_DIRTY           2       // 缓冲区内容被修改
#define BUF_DELAYWRITE      4       // 缓冲区已经被进程释放，但内容还没有写入磁盘
#define BUF_DONE            8      //

typedef struct _BlockBuffer BlockBuffer;
struct _BlockBuffer {
    uint8_t     *bf_data;
    uint16_t    bf_refs;           // 引用计数
    dev_t       bf_dev;            // dev num
    blk_t       bf_blk;            // block num
    uint32_t    bf_status;
    BlockBuffer     *bf_hash_prev;
    BlockBuffer     *bf_hash_next;
    ListEntity      bf_link;
};

error_t init_block_buffer(void);

BlockBuffer *get_block(dev_t dev, blk_t blk);

error_t release_block(BlockBuffer *buf);

void sync_dev(dev_t dev);

#endif // __BUFFER_H__
