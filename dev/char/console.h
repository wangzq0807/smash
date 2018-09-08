#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include "defs.h"

// 颜色表
#define COL_BLACK           0x0
#define COL_BLUE            0x1
#define COL_GREEN           0x2
#define COL_CYAN            0x3
#define COL_RED             0x4
#define COL_PURPLE          0x5
#define COL_BROWN           0x6
#define COL_GRAY            0x7
#define COL_DARK_GRAY       0x8
#define COL_LIGHT_BLUE      0x9
#define COL_LIGHT_GREEN     0xA
#define COL_LIGHT_CYAN      0xB
#define COL_LIGHT_RED       0xC
#define COL_LIGHT_PURPLE    0xD
#define COL_YELLOW          0xE
#define COL_WHITE           0x0F       /* 字符属性(白色) */

void
set_color(int bk, int fg);

void
console_print(const char *buffer);

void
set_cursor(int x, int y);

void
get_cursor(int *x, int *y);

#endif
