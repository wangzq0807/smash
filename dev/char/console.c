#include "console.h"

#define SCREEN_ADR  0xB8000    /* 显存地址 */
#define ONE_LINE    160        /* 一行的空间大小 */
#define ONE_PAGE    0x1000     /* 一页的空间大小 */
#define CHAR_PROP   0x0F       /* 字符属性(白色) */

static uint32_t cursor_pos = 0;
void
console_print(const char *buffer)
{
    while (buffer++) {
        char* next_addr = (char*)(SCREEN_ADR + (cursor_pos << 1));
        if (*buffer == '\n') {
            cursor_pos = (cursor_pos + 80) / 80 * 80;
        }
        if (*buffer == '\r') {
            cursor_pos = (cursor_pos + 80) / 80 * 80;
        }
        else {
            *next_addr = *buffer;
            *(next_addr+1) = CHAR_PROP;
            ++cursor_pos;
        }
        if (cursor_pos >= 2000)
            cursor_pos = 0;
    }
}
