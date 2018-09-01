#include "irq.h"
#include "asm.h"
#include "log.h"
#include "keyboard.h"

int ext_code = 0;

typedef struct _ScanCode   ScanCode;
typedef void (*scan_func)(const ScanCode *sc);

int on_keyboard_handler(IrqFrame *irq);
static void _ascii_code(const ScanCode *sc);
static void _control_code(const ScanCode *sc);

struct _ScanCode {
    uint16_t    sc_code;
    char        sc_noshift_char;
    char        sc_shift_char;
    scan_func   sc_func;
};

ScanCode    scan_code_set[] = {
    { KSC_NONE, 0, 0, NULL },
    { KSC_ESC, 0, 0x27, _ascii_code },
    { KSC_1, '1', '!', _ascii_code },
    { KSC_2, '2', '@', _ascii_code },
    { KSC_NONE, 0, 0, _ascii_code },
    { KSC_NONE, 0, 0, _ascii_code },
    { KSC_NONE, 0, 0, _ascii_code },
    { KSC_NONE, 0, 0, _ascii_code },
    { KSC_NONE, 0, 0, _control_code },
};

void
_ascii_code(const ScanCode *sc)
{
    printx((uint32_t)sc);
    // print("a");
}

void
_control_code(const ScanCode *sc)
{

}

int
init_keyboard()
{
    set_trap_handler(IRQ_KEYBOARD, on_keyboard_handler);
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
    // const char *str = "aaa";
    // printx(code);
    // print(str);

    return 0;
}
