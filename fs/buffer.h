#ifndef __BUFFER_H__
#define __BUFFER_H__
#include "defs.h"

#define BLK_BUFFER          0x100000      // buffer begin: 1M
#define BLK_BUFFER_END      0x500000      // buffer end: 5M
#define BLK_BUFFER_SIZE     1024          // bytes cnt of per cache

// 缓冲区状态
#define BUF_FREE            1       // 空缓冲区，可以被使用
#define BUF_BUSY            2       // 正在读/写磁盘，不允许被任何进程读或写
#define BUF_DIRTY           4       // 缓冲区内容被修改
#define BUF_DELAYWRITE      8       // 缓冲区已经被进程释放，但内容还没有写入磁盘
#define BUF_DONE            16      //

struct BlockBuffer {
    uint8_t *bf_data;
    uint16_t bf_refs;           // 引用计数
    uint16_t bf_dev;            // dev num
    uint32_t bf_blk;            // block num
    uint32_t bf_status;
    struct BlockBuffer *bf_hash_prev;
    struct BlockBuffer *bf_hash_next;
    struct BlockBuffer *bf_prev;
    struct BlockBuffer *bf_next;
};

error_t init_block_buffer(void);

struct BlockBuffer *get_block(uint16_t dev, uint32_t blk);

error_t release_block(struct BlockBuffer *buf);

#endif // __BUFFER_H__
