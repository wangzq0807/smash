#ifndef __TYPES_H__
#define __TYPES_H__
#include "defs.h"

typedef uint64_t        clock_t;
typedef uint32_t        time_t;
typedef uint64_t        useconds_t;
//common
typedef uint32_t        size_t;
typedef uint32_t        ssize_t;
// proc
typedef int32_t         id_t;
typedef int32_t         pid_t;
typedef int32_t         gid_t;
typedef int32_t         uid_t;
// dev
typedef uint16_t        dev_t;
// fs
typedef uint16_t        ino_t;
typedef uint32_t        off_t;
typedef uint16_t        mode_t;
typedef int16_t         nlink_t;

#endif // __TYPES_H__
