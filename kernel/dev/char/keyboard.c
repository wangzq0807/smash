#include "arch/irq.h"
#include "asm.h"
#include "log.h"
#include "keyboard.h"
#include "tty.h"

#define SHIFT_MODE  1
#define CTL_MODE    2
#define ALT_MODE    4

int kbd_mode = 0;
int caps_key = 0;

int on_keyboard_handler(IrqFrame *irq);

#define LSHIFT      0x2A
#define RSHIFT      0x36
#define LCTL        0x1D
#define RCTL        0x9D
#define LALT        0x38

char unshift_set[126] = {
    0, 0x1B, '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '\b', '\t',   // 0xF
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o', 'p', '[', ']', 0xD, 0, 'a', 's',       // 0x1F
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',     // 0x2F
    'b', 'n', 'm', ',', '.', '/', 0, '*',
    0, ' ', 0, 0, 0, 0, 0, 0,                   // 0x3F
    0, 0, 0, 0, 0, 0, 0, '7',
    '8', '9', '-', '4', '5', '6', '+', '1',     // 0x4F
    '2', '3', '0', '.', 0, 0, 0, 0,
};

char shift_set[126] = {
    0, 0x1B, '!', '@', '#', '$', '%', '^',
    '&', '*', '(', ')', '_', '+', '\b', '\t',   // 0xF
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    'O', 'P', '{', '}', 0xD, 0, 'A', 'S',       // 0x1F
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
    '"', '~', 0, '|', 'Z', 'X', 'C', 'V',       // 0x2F
    'B', 'N', 'M', '<', '>', '?', 0, '*',
    0, ' ', 0, 0, 0, 0, 0, 0,                   // 0x3F
    0, 0, 0, 0, 0, 0, 0, '7',
    '8', '9', '-', '4', '5', '6', '+', '1',      // 0x4F
    '2', '3', '0', '.', 0, 0, 0, 0,
};

char alt_set[126] = {

};


int
init_keyboard()
{
    set_trap_handler(IRQ_KEYBOARD, on_keyboard_handler);
    // outb( 0, 0x64);
    return 0;
}

static void
_on_key_press(int code)
{
    char asciicode = 0;
    if (kbd_mode & SHIFT_MODE) {
        asciicode = shift_set[code];
    }
    else if (kbd_mode & ALT_MODE) {
        asciicode = alt_set[code];
    }
    else {
        asciicode = unshift_set[code];
    }
    // 普通ascii码
    if (asciicode != 0) {
        on_tty_intr(asciicode);
        return;
    }

    switch (code) {
        case LSHIFT:
        case RSHIFT:
            kbd_mode |= SHIFT_MODE;
            break;
        case LCTL:
        case RCTL:
            kbd_mode |= CTL_MODE;
            break;
        case LALT:
            kbd_mode |= ALT_MODE;
            break;
        default:
            break;
    }
}

static void
_on_key_release(int code)
{
    switch (code) {
        case LSHIFT:
        case RSHIFT:
            kbd_mode &= ~SHIFT_MODE;
            break;
        case LCTL:
        case RCTL:
            kbd_mode &= ~CTL_MODE;
            break;
        case LALT:
            kbd_mode &= ~ALT_MODE;
            break;
        default:
            break;
    }
}

int
on_keyboard_handler(IrqFrame *irq)
{
    static int ext_code = 0;
    int code;
    code = inb(0x60);
    // TODO : 0xE0和0xE1的按键暂不处理。
    if (ext_code > 0) {
        ext_code -= 1;
        return 0;
    }
    if (code == 0xE0 ) {
        ext_code = 1;
        return 0;
    }
    else if (code == 0xE1) {
        ext_code = 2;
        return 0;
    }

    if (code & 0x80) {
        // 按键释放
        _on_key_release(code & ~0x80);
    }
    else {
        // 按键按下
        _on_key_press(code);
    }

    return 0;
}
