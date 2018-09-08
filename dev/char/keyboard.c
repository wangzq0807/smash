#include "arch/irq.h"
#include "asm.h"
#include "log.h"
#include "keyboard.h"

int ext_code = 0;
int shift_key = 0;
int ctl_key = 0;
int caps_key = 0;

typedef struct _ScanCode   ScanCode;
typedef void (*scan_func)(const ScanCode *sc);

int on_keyboard_handler(IrqFrame *irq);
static void _ascii_press(const ScanCode *sc);
static void _control_press(const ScanCode *sc);
static void _shift_press(const ScanCode *sc);
static void _caps_press(const ScanCode *sc);
static void _function_press(const ScanCode *sc);
static void _numlock_press(const ScanCode *sc);


struct _ScanCode {
    uint16_t    sc_code;
    char        sc_noshift_char;
    char        sc_shift_char;
    scan_func   sc_func;
};

ScanCode    scan_code_set[] = {
    { KSC_NONE, 0, 0, NULL },
    { KSC_ESC, 0, 0x27, _ascii_press },
    { KSC_1, '1', '!', _ascii_press },
    { KSC_2, '2', '@', _ascii_press },
    { KSC_3, '3', '#', _ascii_press },
    { KSC_4, '4', '$', _ascii_press },
    { KSC_5, '5', '%', _ascii_press },
    { KSC_6, '6', '^', _ascii_press },
    { KSC_7, '7', '&', _ascii_press },
    { KSC_8, '8', '*', _ascii_press },
    { KSC_9, '9', '(', _ascii_press },
    { KSC_0, '0', ')', _ascii_press },
    { KSC_SUB, '-', '_', _ascii_press },
    { KSC_EQ, '=', '+', _ascii_press },
    { KSC_BACK, 0x8, 0x8, _ascii_press },
    { KSC_TAB, '\t', '\t', _ascii_press },
    { KSC_Q, 'q', 'Q', _ascii_press },
    { KSC_W, 'w', 'W', _ascii_press },
    { KSC_E, 'e', 'E', _ascii_press },
    { KSC_R, 'r', 'R', _ascii_press },
    { KSC_T, 't', 'T', _ascii_press },
    { KSC_Y, 'y', 'Y', _ascii_press },
    { KSC_U, 'u', 'U', _ascii_press },
    { KSC_I, 'i', 'I', _ascii_press },
    { KSC_O, 'o', 'O', _ascii_press },
    { KSC_P, 'p', 'P', _ascii_press },
    { KSC_LBRACKET, '[', '{', _ascii_press },
    { KSC_RBRACKET, ']', '}', _ascii_press },
    { KSC_ENTER, 0xD, 0xD, _ascii_press },
    { KSC_LCTL, 0, 0, _control_press },
    { KSC_A, 'a', 'A', _ascii_press },
    { KSC_S, 's', 'S', _ascii_press },
    { KSC_D, 'd', 'D', _ascii_press },
    { KSC_F, 'f', 'F', _ascii_press },
    { KSC_G, 'g', 'G', _ascii_press },
    { KSC_H, 'h', 'H', _ascii_press },
    { KSC_J, 'j', 'J', _ascii_press },
    { KSC_K, 'k', 'K', _ascii_press },
    { KSC_L, 'l', 'L', _ascii_press },
    { KSC_SEMICOLON, ';', ':', _ascii_press },
    { KSC_QUOTE, '\'', '"', _ascii_press },
    { KSC_TICK, '`', '~', _ascii_press },
    { KSC_LSHIFT, 0, 0, _shift_press },
    { KSC_BACKSLANT, '\\', '|', _ascii_press },
    { KSC_Z, 'z', 'Z', _ascii_press },
    { KSC_X, 'x', 'X', _ascii_press },
    { KSC_C, 'c', 'C', _ascii_press },
    { KSC_V, 'v', 'V', _ascii_press },
    { KSC_B, 'b', 'B', _ascii_press },
    { KSC_N, 'n', 'N', _ascii_press },
    { KSC_M, 'm', 'M', _ascii_press },
    { KSC_COMMA, ',', '<', _ascii_press },
    { KSC_FULLSTOP, '.', '>', _ascii_press },
    { KSC_SLANT, '/', '?', _ascii_press },
    { KSC_RSHIFT, 0, 0, _shift_press },
    { KSC_ASTERISK, 0x2A, 0x2A, _ascii_press },
    { KSC_LALT, 0, 0, _control_press },
    { KSC_SPACE, ' ', ' ', _ascii_press },
    { KSC_CAP, 0, 0, _caps_press },
    { KSC_F1, 0, 0, _function_press },
    { KSC_F2, 0, 0, _function_press },
    { KSC_F3, 0, 0, _function_press },
    { KSC_F4, 0, 0, _function_press },
    { KSC_F5, 0, 0, _function_press },
    { KSC_F6, 0, 0, _function_press },
    { KSC_F7, 0, 0, _function_press },
    { KSC_F8, 0, 0, _function_press },
    { KSC_F9, 0, 0, _function_press },
    { KSC_F10, 0, 0, _function_press },
    { KSC_NUMLOCK, 0, 0, _numlock_press },
    { KSC_SCROLL, 0, 0, NULL },
    { KSC_K7, '7', '7', _ascii_press },
    { KSC_K8, '8', '8', _ascii_press },
    { KSC_K9, '9', '9', _ascii_press },
    { KSC_KSUB, '-', '-', _ascii_press },
    { KSC_K4, '4', '4', _ascii_press },
    { KSC_K5, '5', '5', _ascii_press },
    { KSC_K6, '6', '6', _ascii_press },
    { KSC_KPLUS, '+', '+', _ascii_press },
    { KSC_K1, '1', '1', _ascii_press },
    { KSC_K2, '2', '2', _ascii_press },
    { KSC_K3, '3', '3', _ascii_press },
    { KSC_K0, '4', '4', _ascii_press },
    { KSC_KFULLSTOP, '.', '.', _ascii_press },
};

int
init_keyboard()
{
    set_trap_handler(IRQ_KEYBOARD, on_keyboard_handler);
    outb( 0, 0x64);
    return 0;
}

int
on_keyboard_handler(IrqFrame *irq)
{
    uint16_t code;
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

    const int scan_code_num = sizeof(scan_code_set) / sizeof(ScanCode);
    if (code < scan_code_num) {
        const ScanCode *sc = &scan_code_set[code];
        if (sc->sc_func != NULL) {
            sc->sc_func(sc);
        }
    }

    return 0;
}

static void
_ascii_press(const ScanCode *sc)
{
    char str[2];
    str[0] = sc->sc_noshift_char;
    str[1] = 0;
    printk(str);
}

static void
_control_press(const ScanCode *sc)
{
    // printx((uint32_t)sc);
}

// static void
// _control_release(const ScanCode *sc)
// {
//     printx((uint32_t)sc);
// }

static void
_shift_press(const ScanCode *sc)
{

}

// static void
// _shift_release(const ScanCode *sc)
// {

// }

static void
_caps_press(const ScanCode *sc)
{

}

static void
_function_press(const ScanCode *sc)
{

}

static void
_numlock_press(const ScanCode *sc)
{

}

