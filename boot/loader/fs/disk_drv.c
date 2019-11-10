#include "asm.h"
#include "string.h"
#include "disk_drv.h"
#include "log.h"

#define NUMSECS_OF_BLK       (BLOCK_SIZE / SECTOR_SIZE)

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

uint8_t block_buf[BLOCK_SIZE];

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
        outb(ATA_REG_DRV, drv_mode);            // 选择M/S驱动器，选择LBA/CHS模式
        // TODO: wait for 400ns
    outb(ATA_REG_COUNT, cnt);                   // 扇区数(0-255, 0表示传输256个扇区)
    outb(ATA_REG_LBA1, BYTE1(lba_addr));        // LBA 0-7
    outb(ATA_REG_LBA2, BYTE2(lba_addr));        // LBA 8-15
    outb(ATA_REG_LBA3, BYTE3(lba_addr));        // LBA 16-23
    outb(ATA_REG_CMD, cmd);                     // 发送命令到驱动器
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

// 等待驱动器能传输数据
static int
ata_wait_ready() 
{
    while (1) {
        const uint8_t status = inb(ATA_REG_STATUS);
        if ( !(status &ATA_STATUS_BUSY)
            && (status & ATA_STATUS_DRQ))
            return 0;
    }
    return 1;
}

void*
ata_read(uint32_t blk_addr)
{
    const uint32_t lba_addr = blk_addr * NUMSECS_OF_BLK;
    ata_cmd(lba_addr, NUMSECS_OF_BLK, ATA_CMD_READ);
    int nread = 0;
    while (nread < NUMSECS_OF_BLK) {
        if (ata_wait_ready() != 0)
            return NULL;
        insw(ATA_REG_DATA, block_buf+(nread*SECTOR_SIZE), SECTOR_SIZE/2);
        nread++;
    }
    return block_buf;
}

int
ata_write(uint32_t blk_addr, void* buffer)
{
    const uint32_t lba_addr = blk_addr * NUMSECS_OF_BLK;
    ata_cmd(lba_addr, NUMSECS_OF_BLK, ATA_CMD_READ);
    int nwrite = 0;
    while (nwrite < NUMSECS_OF_BLK) {
        if (ata_wait_ready() != 0)
            return -1;
        outsw(ATA_REG_DATA, buffer+(nwrite*SECTOR_SIZE), SECTOR_SIZE/2);
        nwrite++;
    }
    return 0;
}

