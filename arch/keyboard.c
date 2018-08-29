#include "irq.h"
#include "asm.h"
#include "log.h"

int on_keyboard_handler(IrqFrame *irq);

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

    printxw(code);

    return 0;
}
