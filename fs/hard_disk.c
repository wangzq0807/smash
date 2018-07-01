#include "asm.h"
#include "memory.h"
#include "arch/arch.h"
#include "hard_disk.h"
#include "log.h"
#include "fs.h"

struct DiskRequest {
    struct BlockBuffer  *dr_buf;
    uint32_t            dr_cmd;
    struct DiskRequest  *dr_next;
};

struct DistReqHead {
    uint32_t            dr_lock;
    struct DiskRequest  *dr_req;
};

struct DistReqHead disk_queue;
struct DiskRequest *disk_queue_tail = NULL;
#define QUEUE_COUNT (PAGE_SIZE / sizeof(struct DiskRequest))
#define BYTE_PER_BLK            (1 << BLOCK_LOG_SIZE)
#define PER_BLOCK_SECTORS       (BYTE_PER_BLK / SECTOR_SIZE)

// ATA 寄存器(Primary Bus, Master Drives)
#define ATA_REG_DATA        0x1F0   // 数据端口
#define ATA_REG_FEATURES    0x1F1   // 
#define ATA_REG_COUNT       0x1F2   // 读写扇区数
#define ATA_REG_LBA1        0x1F3
#define ATA_REG_LBA2        0x1F4
#define ATA_REG_LBA3        0X1F5
#define ATA_REG_DRV         0x1F6   // 选择M/S驱动器
#define ATA_REG_CMD         0x1F7   // 命令端口
#define ATA_REG_STATUS      0x3F6   // 状态端口
// ATA command set
#define ATA_CMD_READ    0x20        // 命令：读取
#define ATA_CMD_WRITE   0x30        // 命令：写入
// ATA status
#define ATA_STATUS_ERR      0x1     // 发生错误
#define ATA_STATUS_INDEX    0x2     // (已废弃不用)
#define ATA_STATUS_ECC      0x4     // (已废弃不用)
#define ATA_STATUS_DRQ      0x8     // 已准备就绪，有数据待传输，与BUSY互斥
#define ATA_STATUS_SEEK     0x10    // 
#define ATA_STATUS_FAULT    0x20    // 故障（除了ERR之外的，其他任何错误）
#define ATA_STATUS_READY    0x40    // 能处理接收到的命令，没有发生错误
#define ATA_STATUS_BUSY     0x80    // 正在准备发送/接口数据

extern void on_disk_intr();
static int do_request();

int
init_disk()
{
    // 设置中磁盘中断
    set_intr_gate(INTR_DISK, on_disk_intr);
    // 初始化磁盘请求队列
    struct DiskRequest *req = alloc_page();
    for (int i = 0; i < QUEUE_COUNT; ++i) {
        req[i].dr_next = &req[i+1];
        req[i].dr_cmd = 0;
        req[i].dr_buf = NULL;
    }
    req[QUEUE_COUNT-1].dr_next = &req[0];
    disk_queue_tail = req;
    disk_queue.dr_lock = 0;
    disk_queue.dr_req = req;
    return 0;
}

int
ata_cmd(uint32_t lba_addr, uint8_t cnt, uint8_t cmd)
{
    // TODO: check drv status
    static uint8_t last_drv = 0;
    // 选择M/S驱动器，选择LBA/CHS模式
    // 高4位 : | obs | LBA | obs | drv |
    // 低4位 : | LBA (27 - 24) |
    const uint8_t drv_mode = 0xE0 | BYTE4(lba_addr);
    // op ata disk by LBA-28bit-mode
    lba_addr = lba_addr & 0xffffff;             // 28bit
    if (last_drv != drv_mode)
        outb(drv_mode, ATA_REG_DRV);            // 选择M/S驱动器，选择LBA/CHS模式
        // TODO: wait for 400ns
    outb(cnt, ATA_REG_COUNT);                   // 扇区数(0-255, 0表示传输256个扇区)
    outb(BYTE1(lba_addr), ATA_REG_LBA1);        // LBA 0-7
    outb(BYTE2(lba_addr), ATA_REG_LBA2);        // LBA 8-15
    outb(BYTE3(lba_addr), ATA_REG_LBA3);        // LBA 16-23
    outb(cmd, ATA_REG_CMD);                     // 发送命令到驱动器
    return 0;
}

// 检查驱动器是否能接收命令
int
ata_is_ready()
{
    const uint8_t status = inb(ATA_REG_STATUS);
    if (!(status & ATA_STATUS_BUSY)
        && (status & ATA_STATUS_READY))
        return 1;
    return 0;
}

// 检查驱动器是否能传输数据
static int
ata_wait_ready() 
{
    int cnt = 100000;
    while (--cnt) {
        const uint8_t status = inb(ATA_REG_STATUS);
        if ( !(status &ATA_STATUS_BUSY)
            && (status & ATA_STATUS_DRQ))
            return 0;
    }
    return 1;
}

int
ata_read(struct BlockBuffer *buffer)
{
    struct DiskRequest *req = disk_queue_tail;
    req->dr_buf = buffer;
    req->dr_cmd = ATA_CMD_READ;
    disk_queue_tail = disk_queue_tail->dr_next;

    if (do_request() == -1)
        return -1;

    return 0;
}

int
ata_write(struct BlockBuffer *buffer)
{
    struct DiskRequest *req = disk_queue_tail;
    req->dr_buf = buffer;
    req->dr_cmd = ATA_CMD_WRITE;
    disk_queue_tail = disk_queue_tail->dr_next;

    if (do_request() == -1)
        return -1;

    return 0;
}

static int
do_request()
{
    // 请求队列头加锁
    if (lock(&disk_queue.dr_lock) != 0)
        return 0;
    struct DiskRequest *req = disk_queue.dr_req;
    // TODO : 电梯算法以后再实现
    if (req == disk_queue_tail || req->dr_buf == NULL) {
        unlock(&disk_queue.dr_lock);
        return -1;
    }
    struct BlockBuffer *buffer = req->dr_buf;
    const uint32_t lba_addr = buffer->bf_blk * PER_BLOCK_SECTORS;
    const uint8_t cnt = PER_BLOCK_SECTORS;
    const uint8_t cmd = req->dr_cmd;
    ata_cmd(lba_addr, cnt, cmd);

    if (cmd == ATA_CMD_WRITE) {
        ata_wait_ready();
        // begin write
        outsw(buffer->bf_data, BYTE_PER_BLK/2, ATA_REG_DATA);
    }
    unlock(&disk_queue.dr_lock);
    return 0;
}

void
on_disk_handler()
{
    /* 设置8259A的OCW2,发送结束中断命令 */
    outb(0x20, 0x20);
    outb(0x20, 0xA0);

    struct DiskRequest *req = disk_queue.dr_req;
    struct BlockBuffer *buffer = req->dr_buf;

    if (req->dr_cmd == ATA_CMD_READ)
        insw(BYTE_PER_BLK/2, ATA_REG_DATA, buffer->bf_data);
    // 释放/解锁缓冲区
    if (buffer->bf_status & BUF_BUSY) {
        // 不需要加锁，因为BUSY时，我们只有在这里才修改bf_status的值
        buffer->bf_status = BUF_FREE;
        // TODO:必须加锁来保证freelist不会被破坏，似乎没有更好的办法
        release_block(buffer);
    }

    disk_queue.dr_req = req->dr_next;
    unlock(&disk_queue.dr_lock);
    do_request();
}
