#ifndef __LOG_H__
#define __LOG_H__
#include "types.h"

typedef char* va_list;
#define ROUND_INT_SIZE(n)  ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))

#define va_start(ap, prev) (ap = (va_list)&prev + ROUND_INT_SIZE(prev))
#define va_arg(ap, type)                    \
({                                          \
    ap += ROUND_INT_SIZE(type);             \
    *(type *)(ap - ROUND_INT_SIZE(type));   \
})
#define va_end(ap)

typedef enum _LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
} LogLevel;

int
log_write(LogLevel, const char *fmt, ...);

#define KLOG(level, format, ...) { \
    log_write(level, format,  ##__VA_ARGS__);   \
}

#endif