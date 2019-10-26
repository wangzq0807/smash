#ifndef __DEFS_H__
#define __DEFS_H__

typedef unsigned char   uint8_t;
typedef char            int8_t;
typedef unsigned short  uint16_t;
typedef short           int16_t;
typedef unsigned long   uint32_t;
typedef long            int32_t;
typedef unsigned long long uint64_t;
typedef long long       int64_t;

typedef int32_t         error_t;

//common
typedef uint32_t        size_t;
typedef uint32_t        ssize_t;

// mem
typedef size_t*         pdt_t;      // 页目录表
typedef size_t          pde_t;      // 页目录表项
typedef pde_t*          pt_t;       // 页表
typedef size_t          pte_t;      // 页表项
typedef size_t          vm_t;   // 线性地址
// fs
typedef uint64_t        lba_t;
typedef uint32_t        blk_t;
typedef uint32_t        zone_t;

#define NO_ERROR    (0)

#define BYTE1(val) ((val) & 0xFF)
#define BYTE2(val) ((val) >> 8 & 0xFF)
#define BYTE3(val) ((val) >> 16 & 0xFF)
#define BYTE4(val) ((val) >> 24 & 0xFF)

#define MIN(a, b)   ((a) > (b) ? (b) : (a))
#define MAX(a, b)   ((a) > (b) ? (a) : (b))

#define NULL    0

#endif // __DEFS_H__