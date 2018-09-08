#include "log.h"
#include "dev/char/console.h"

#define COPY_MODE   0
#define FMT_MODE    1

#define HEX_FMT     1
#define INT_FMT     2
#define UINT_FMT    3

char *
num2str(char *buf, int num, int flags)
{
    if (flags == HEX_FMT) {
        *buf++ = '0';
        *buf++ = 'x';
    }
    char tmpbuf[30];
    char asciinum[] = "0123456789ABCDEF";
    int i = 0;

    if (flags == HEX_FMT) {
        while (num > 0) {
            int tmp = num & 0xF;
            num = num >> 4;
            tmpbuf[i++] = asciinum[tmp];
        }
    }
    else if (flags == INT_FMT) {
        while (num > 0) {
            int tmp = num % 10;
            num = num / 10;
            tmpbuf[i++] = asciinum[tmp];
        }
    }

    while(i > 0) {
        *buf++ = tmpbuf[--i];
    }

    return buf;
}

char printbuf[256];
void
printk(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsprintf(printbuf, fmt, args);
    console_print(printbuf);
    va_end(args);
}

void
vsprintf(char *buf, const char *fmt, va_list args)
{
    int mode = COPY_MODE;
    for(; *fmt; ++fmt) {
        if (mode == COPY_MODE) {
            if (*fmt == '%') {
                mode = FMT_MODE;
                continue;
            }
            *buf++ = *fmt;
        }
        else if (mode == FMT_MODE) {
            switch (*fmt) {
                case 'X':
                case 'x':
                    buf = num2str(buf, va_arg(args, int), HEX_FMT);
                    break;
                case 'd':
                case 'i':
                    buf = num2str(buf, va_arg(args, int), INT_FMT);
                    break;
                default:
                    break;
            }
            mode = COPY_MODE;
        }
    }
    *buf = '\0';
}