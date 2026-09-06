#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

#define KEY_UP        0x80
#define KEY_DOWN      0x81
#define KEY_LEFT      0x82
#define KEY_RIGHT     0x83

#define CTRL_C        0x03
#define CTRL_Z        0x1A
#define CTRL_S        0x13
#define CTRL_Q        0x11
#define ALT_SHIFT_J   0x1E

char keyboard_read_char(void);

#endif