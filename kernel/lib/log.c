#include "lib/log.h"
#include "asm.h"
#include "arch/task.h"
#include "dev/char/serial.h"

const LogLevel log_level = KLOG_LEVEL;

#define COPY_MODE   0
#define FMT_MODE    1

#define HEX_FMT     1
#define INT_FMT     2
#define UINT_FMT    3

char *
num2str(char *buf, int num, int flags)
{
    char tmpbuf[30];
    char asciinum[] = "0123456789ABCDEF";
    int i = 0;

    if (flags == HEX_FMT) {
        uint32_t hexnum = (uint32_t)num;
        while (hexnum > 0) {
            int tmp = hexnum & 0xF;
            hexnum = hexnum >> 4;
            tmpbuf[i++] = asciinum[tmp];
        }
        if (i == 0)
            tmpbuf[i++] = '0';
    }
    else if (flags == INT_FMT) {
        while (num > 0) {
            int tmp = num % 10;
            num = num / 10;
            tmpbuf[i++] = asciinum[tmp];
        }
        if (i == 0)
            tmpbuf[i++] = '0';
    }

    while(i > 0) {
        *buf++ = tmpbuf[--i];
    }

    return buf;
}

int
printk(const char *fmt, ...)
{
    char printbuf[256];
    volatile va_list args;  // Note: 必须volatile,否则可能被优化为inline
    va_start(args, fmt);
    int len = vsprintf(printbuf, fmt, args);
    const char* tmp = printbuf;
    while (*tmp != '\0')
        serial_write(COM_PORT1, *tmp++);
    va_end(args);
    return len;
}

int
sprintf(char *str, const char *fmt, ...)
{
    volatile va_list args;  // Note: 必须volatile,否则可能被优化为inline
    va_start(args, fmt);
    int len = vsprintf(str, fmt, args);
    va_end(args);
    return len;
}

int
vsprintf(char *buf, const char *fmt, va_list args)
{
    char* iter = buf;
    int mode = COPY_MODE;
    for(; *fmt; ++fmt) {
        if (mode == COPY_MODE) {
            if (*fmt == '%') {
                mode = FMT_MODE;
                continue;
            }
            *iter++ = *fmt;
        }
        else if (mode == FMT_MODE) {
            switch (*fmt) {
                case 'X':
                case 'x':
                    iter = num2str(iter, va_arg(args, int), HEX_FMT);
                    break;
                case 'd':
                case 'i':
                    iter = num2str(iter, va_arg(args, int), INT_FMT);
                    break;
                case 'c':
                    *iter++ = va_arg(args, char);
                    break;
                case 's':
                {
                    char *src = va_arg(args, char*);
                    while (*src) {
                        *iter++ = *src++;
                    }
                    break;
                }
                default:
                    break;
            }
            mode = COPY_MODE;
        }
    }
    *iter = '\0';
    return iter - buf;
}

int
log_write(LogLevel lv, const char *fmt, ...)
{
    char printbuf[256];
    volatile va_list args;
    va_start(args, fmt);
    vsprintf(printbuf, fmt, args);
    va_end(args);

    const char* szlevel[4] = {"D", "I", "W", "E"};
    int pid = 0;
    Task *ts = current_task();
    if (ts != NULL)
        pid = ts->ts_pid;
    printk("[%s|%d] %s\n", szlevel[lv], pid, printbuf);
    return 0;
}