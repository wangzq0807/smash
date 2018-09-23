
#ifndef __STAT_H__
#define __STAT_H__
#include "types.h"

struct stat {
    dev_t   st_dev;
    ino_t   st_ino;
    mode_t  st_mode;
    nlink_t st_nlink;
    off_t   st_size;
    time_t  st_atime;
    time_t  st_mtime;
    time_t  st_ctime;
};
/************
 * 文件权限
 ************/
#define S_IXOTH     0000001         // 允许其他用户执行文件
#define S_IWOTH     0000002         // 允许其他用户写文件
#define S_IROTH     0000004         // 允许其他用户读文件
#define S_IRWXO     0000007         // 允许其他用户读，写，执行文件

#define S_IXGRP     0000010         // 允许同组用户执行文件
#define S_IWGRP     0000020         // 允许同组用户写文件
#define S_IRGRP     0000040         // 允许同组用户读文件
#define S_IRWXG     0000070         // 允许同组用户读，写，执行文件

#define S_IXUSR     0000100         // 允许拥有者执行文件
#define S_IWUSR     0000200         // 允许拥有者写文件
#define S_IRUSR     0000400         // 允许拥有者读文件
#define S_IRWXU     0000700         // 允许拥有者读，写，执行文件

/************
 * 文件类型
 ************/
#define S_IFIFO     0010000         // FIFO 文件
#define S_IFCHR     0020000         // 字符文件
#define S_IFDIR     0040000         // 目录文件
#define S_IFBLK     0060000         // 块文件
#define S_IFREG     0100000         // 普通文件
#define S_IFLNK     0120000         // 符号文件
#define S_IFSOCK    0140000         // 套接字文件

#define S_IFMT      0170000         // 文件属性中，文件类型部分的MASK
#define S_ISFIFO(m)     (((m) & S_IFMT) == S_IFIFO)
#define S_ISCHR(m)      (((m) & S_IFMT) == S_IFCHR)
#define S_ISDIR(m)      (((m) & S_IFMT) == S_IFDIR)
#define S_ISBLK(m)      (((m) & S_IFMT) == S_IFBLK)
#define S_ISREG(m)      (((m) & S_IFMT) == S_IFREG)
#define S_ISLNK(m)      (((m) & S_IFMT) == S_IFLNK)
#define S_ISSOCK(m)     (((m) & S_IFMT) == S_IFSOCK)

#endif // __STAT_H__