#include "log.h"

#define SCREEN_ADR  0xB8000    /* 显存地址 */
#define ONE_LINE    160        /* 一行的空间大小 */
#define ONE_PAGE    0x1000     /* 一页的空间大小 */
#define CHAR_PROP   0x0F       /* 字符属性(白色) */
uint32_t cursor_pos = 0;
void
print(const char *buffer)
{
    for (uint32_t i = 0; i < 0xffff; ++i) {
        char* next_addr = (char*)(SCREEN_ADR + (cursor_pos << 1));
        if (buffer[i] == '\n') {
            cursor_pos = (cursor_pos + 80) / 80 * 80;
        }
        else if (buffer[i] == '\0') {
            return ;
        }
        else {
            *next_addr = buffer[i];
            *(next_addr+1) = CHAR_PROP;
            ++cursor_pos;
        }
        if (cursor_pos >= 2000)
            cursor_pos = 0;
    }
}

void
printx(uint32_t val)
{
    char buffer[] = { "0x00000000 " };
    for (int32_t i = 0; i < 8; ++i) {
        int32_t hex = val & 0xF;
        if (hex > 9)
            buffer[9-i] = (0x41 - 10) + hex;
        else
            buffer[9-i] = 0x30 + hex;
        val = val >> 4;
    }
    print(buffer);
}

void
printxw(uint16_t val)
{
    char buffer[] = { "0x0000 " };
    for (int32_t i = 0; i < 4; ++i) {
        int32_t hex = val & 0xF;
        if (hex > 9)
            buffer[5-i] = (0x41 - 10) + hex;
        else
            buffer[5-i] = 0x30 + hex;
        val = val >> 4;
    }
    print(buffer);
}
