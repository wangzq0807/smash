

#include "lib/log.h"
#include "stdio.h"

const LogLevel log_level = (LogLevel)KLOG_LEVEL;

int
log_write(LogLevel lv, const char *fmt, ...)
{
    char printbuf[256];
    va_list args;
    va_start(args, fmt);
    vsprintf(printbuf, fmt, args);
    va_end(args);

    const char* szlevel[4] = {"D", "I", "W", "E"};
    printf("[%s] %s\n", szlevel[lv], printbuf);
    return 0;
}