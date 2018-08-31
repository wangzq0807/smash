#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

// Scan Code
#define KSC_NONE    0x0
#define KSC_ESC     0x1
#define KSC_1       0x2
#define KSC_2       0x3
#define KSC_3       0x4
#define KSC_4       0x5
#define KSC_5       0x6
#define KSC_6       0x7
#define KSC_7       0x8
#define KSC_8       0x9
#define KSC_9       0xA
#define KSC_0       0XB
#define KSC_SUB     0xC     // -
#define KSC_EQ      0xD     // =
#define KSC_BACK    0xE     // backspace
#define KSC_TAB     0xF     // tab
#define KSC_Q       0x10
#define KSC_W       0x11
#define KSC_E       0x12
#define KSC_R       0x13
#define KSC_T       0x14
#define KSC_Y       0x15
#define KSC_U       0x16
#define KSC_I       0x17
#define KSC_O           0x18
#define KSC_P           0x19
#define KSC_LBRACKET    0x1A    // left bracket
#define KSC_RBRACKET    0x1B    // right bracket

int
init_keyboard();

#endif // __KEYBOARD_H__