#include "console.h"
#include "asm.h"
#include "string.h"
#include "memory.h"

uint32_t cursor_pos = 0;
uint8_t bk_color = COL_BLACK;
uint8_t fg_color = COL_GRAY;

typedef struct _VgaChar VgaChar;
struct _VgaChar
{
    char    vc_char;
    uint8_t vc_color;
};

static void _update_cursor();
static void _clear_line(int ln);
static void _scroll_down(int nline);

#define SCREEN_ADR          0xB8000     /* 显存地址 */
#define CHARS_PER_LINE      80          /* 一行的字符数 */
#define LINES_PER_PAGE      25          /* 一页的行数 */
#define ONE_PAGE    (CHARS_PER_LINE * LINES_PER_PAGE)

#define VGA_ROM()   ((VgaChar *)(pym2vm(SCREEN_ADR)))
#define CUR_COLOR() (bk_color << 4 | fg_color)

void
set_color(int bk, int fg)
{
    bk_color = (uint8_t)(bk & 0xF);
    fg_color = (uint8_t)(fg & 0xF);
}

void
console_print(const char *buffer)
{
    while(*buffer) {
        VgaChar* next_addr = VGA_ROM() + cursor_pos;
        switch(*buffer) {
            case '\n':
            case '\r':
            {
                int nextln = cursor_pos / CHARS_PER_LINE + 1;
                cursor_pos = nextln * CHARS_PER_LINE;
                _clear_line(nextln);
                break;
            }
            case '\b':
            {
                if (cursor_pos > 0)
                    cursor_pos -= 1;
                VgaChar* curchar = VGA_ROM() + cursor_pos;
                curchar->vc_char = ' ';
                curchar->vc_color = CUR_COLOR();
                break;
            }
            default:
            {
                next_addr->vc_char = *buffer;
                next_addr->vc_color = CUR_COLOR();
                ++cursor_pos;
                break;
            }
        }
        buffer++;
    }
    int curline = cursor_pos / CHARS_PER_LINE;
    if (curline > (LINES_PER_PAGE - 1)) {
        int scrline = curline - LINES_PER_PAGE + 1;
        _scroll_down(scrline);
    }
    _update_cursor();
}

void
set_cursor(int x, int y)
{
    y = y % LINES_PER_PAGE;
    x = x % CHARS_PER_LINE;
    cursor_pos = y * CHARS_PER_LINE + x;
    _update_cursor();
}

void
get_cursor(int *x, int *y)
{
    *x = cursor_pos % CHARS_PER_LINE;
    *y = cursor_pos / CHARS_PER_LINE;
}

static void
_scroll_down(int nline)
{
    VgaChar* dst_chars = VGA_ROM();
    VgaChar* src_chars = VGA_ROM() + CHARS_PER_LINE * nline;
    for (int i = nline; i < LINES_PER_PAGE + nline; ++i) {
        for (int j = 0; j < CHARS_PER_LINE; ++j)
            *dst_chars++ = *src_chars++;
    }
    cursor_pos -= CHARS_PER_LINE * nline;
}

static void
_update_cursor()
{
    outb(0x3d4, 0xe);
    outb(0x3d5, (uint8_t)(cursor_pos >> 8));
    outb(0x3d4, 0xf);
    outb(0x3d5, (uint8_t)(cursor_pos & 0xff));
}

void
_clear_line(int ln)
{
    int start = ln * CHARS_PER_LINE;
    VgaChar blank_char = { ' ', CUR_COLOR() };
    VgaChar* linechars = VGA_ROM() + start;
    for (int i = 0; i < CHARS_PER_LINE; ++i) {
        linechars[i] = blank_char;
    }
}
