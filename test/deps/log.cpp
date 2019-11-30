

#include "lib/log.h"

const LogLevel log_level = (LogLevel)KLOG_LEVEL;

int
log_write(LogLevel lv, const char *fmt, ...)
{
    return 0;
}