#ifndef __LOG_H__
#define __LOG_H__
#include "sys/types.h"
#include "stdarg.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef enum _LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
} LogLevel;

extern const LogLevel log_level;

int
printk(const char *fmt, ...);

int
log_write(LogLevel, const char *fmt, ...);

#ifdef KLOG_ENABLE
    #define KLOG(level, format, ...) { \
        if (level >= log_level) \
            log_write(level, format,  ##__VA_ARGS__);   \
    }
#else
    #define KLOG(level, format, ...) {}
#endif // KLOG_ENABLE

#ifdef BOCHS_IODEBUG
static inline void bochs_break() {
    __asm__ volatile("xchg %bx, %bx");
}
#else
static inline void bochs_break() {
}
#endif // BOCHS_IODEBUG

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __LOG_H__