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
#define KSC_ENTER       0x1C
#define KSC_LCTL        0x1D
#define KSC_A           0x1E
#define KSC_S           0x1F
#define KSC_D           0x20
#define KSC_F           0x21
#define KSC_G           0x22
#define KSC_H           0x23
#define KSC_J           0x24
#define KSC_K           0x25
#define KSC_L           0x26
#define KSC_SEMICOLON   0x27    // ;
#define KSC_QUOTE       0x28    // '
#define KSC_TICK        0x29    // `
#define KSC_LSHIFT      0x2A
#define KSC_BACKSLANT   0x2B    //
#define KSC_Z           0x2C    //
#define KSC_X           0x2D
#define KSC_C           0x2E
#define KSC_V           0x2F
#define KSC_B           0x30
#define KSC_N           0x31
#define KSC_M           0x32
#define KSC_COMMA       0x33    // ,
#define KSC_FULLSTOP    0x34    // .
#define KSC_SLANT       0x35    // /
#define KSC_RSHIFT      0x36
#define KSC_ASTERISK    0x37
#define KSC_LALT        0x38
#define KSC_SPACE       0x39
#define KSC_CAP         0x3A
#define KSC_F1          0x3B
#define KSC_F2          0x3C
#define KSC_F3          0x3D
#define KSC_F4          0x3E
#define KSC_F5          0x3F
#define KSC_F6          0x40
#define KSC_F7          0x41
#define KSC_F8          0x42
#define KSC_F9          0x43
#define KSC_F10         0x44
#define KSC_NUMLOCK     0x45
#define KSC_SCROLL      0x46
#define KSC_K7          0x47
#define KSC_K8          0x48
#define KSC_K9          0x49
#define KSC_KSUB        0x4A
#define KSC_K4          0x4B
#define KSC_K5          0x4C
#define KSC_K6          0x4D
#define KSC_KPLUS       0x4E
#define KSC_K1          0x4F
#define KSC_K2          0x50
#define KSC_K3          0x51
#define KSC_K0          0x52
#define KSC_KFULLSTOP   0x53

int
init_keyboard();

#endif // __KEYBOARD_H__