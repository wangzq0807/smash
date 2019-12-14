#ifndef __FSDEFS_H__
#define __FSDEFS_H__

#define ROOT_DEVICE    0

#define SECTOR_SIZE             512         // 一个扇区大小
#define BLOCK_SHIFT          10          // 每块的log大小
#define SUPER_BLOCK_BEGAIN      1           // 超级块在分区中的开始块号
#define SUPER_BLOCK_SIZE        1           // 一个超级块占的块数
#define MINIX_V2                0x2478      // minix v2文件系统标识

#define MAX_IMAP_NUM            8           // inode位图最大区块数
#define MAX_ZMAP_NUM            256         // znode位图最大区块数

#define DIRECT_ZONE             7           // 直接索引区块数
#define NUMBER_ZONE             10          // 总的索引区块数
#define FILENAME_LEN            29          // 文件名长度

#define ROOT_INODE              1           // 根节点
#define INVALID_ZONE            0
#define INVALID_INODE           0


#define BLOCK_SIZE     (1 << BLOCK_SHIFT)

#endif // __FSDEFS_H__