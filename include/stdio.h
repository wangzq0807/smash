#ifndef __STDIO_H__
#define __STDIO_H__

#define stdin 0
#define stdout 1

int
printf(const char *format, ...);

int
sprintf(char *str, const char *format, ...);

#endif
