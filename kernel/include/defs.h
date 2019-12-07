#ifndef __DEFS_H__
#define __DEFS_H__

#define BOOL    int
#define FALSE   0
#define TRUE    1

#define NO_ERROR    (0)

#define BYTE1(val) ((val) & 0xFF)
#define BYTE2(val) ((val) >> 8 & 0xFF)
#define BYTE3(val) ((val) >> 16 & 0xFF)
#define BYTE4(val) ((val) >> 24 & 0xFF)

#define MIN(a, b)   ((a) > (b) ? (b) : (a))
#define MAX(a, b)   ((a) > (b) ? (a) : (b))

#endif // __DEFS_H__