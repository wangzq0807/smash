#ifndef __DISK_DRV__
#define __DISK_DRV__
#include "types.h"

#define SECTOR_SIZE             512         // 一个扇区大小
#define BLOCK_SHIFT          10          // 每块的log大小
#define BLOCK_SIZE              (1 << BLOCK_SHIFT)

// ATA registers(Primary Bus)
#define ATA_REG_DATA        0x1F0
#define ATA_REG_FEATURES    0x1F1
#define ATA_REG_COUNT       0x1F2
#define ATA_REG_LBA1        0x1F3
#define ATA_REG_LBA2        0x1F4
#define ATA_REG_LBA3        0X1F5
#define ATA_REG_DRV         0x1F6
#define ATA_REG_CMD         0x1F7
#define ATA_REG_STATUS      0x3F6
// ATA command set
#define ATA_CMD_READ    0x20    // read command
#define ATA_CMD_WRITE   0x30    // write command
// ATA status
#define ATA_STATUS_ERR      0x1
#define ATA_STATUS_INDEX    0x2
#define ATA_STATUS_ECC      0x4
#define ATA_STATUS_DRQ      0x8
#define ATA_STATUS_SEEK     0x10
#define ATA_STATUS_FAULT    0x20
#define ATA_STATUS_READY    0x40
#define ATA_STATUS_BUSY     0x80

#define IDE0_MAJOR          3
#define HD_MAJOR            IDE0_MAJOR

// 读磁盘: 读取一个block
void* ata_read(uint32_t blk_addr);
// 写磁盘: 写入一个block
int ata_write(uint32_t blk_addr, void* buffer);

#endif
