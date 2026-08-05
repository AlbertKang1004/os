#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KBD_DATA_PORT        0x60
#define KBD_COMMAND_PORT     0x64
#define MOD_LSHIFT  (1 << 0)
#define MOD_RSHIFT  (1 << 1)
#define MOD_LCTRL   (1 << 2)
#define MOD_RCTRL   (1 << 3)
#define MOD_LALT    (1 << 4)
#define MOD_RALT    (1 << 5)
#define MOD_CAPS    (1 << 6)

unsigned char read_scan_code(void);
void keyboard_init(void);
int keyboard_read(void);

#endif