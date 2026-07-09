#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KBD_DATA_PORT        0x60

/** read_scan_code:
 *  Reads a scan code from the keyboard
 *
 *  @return The scan code (NOT an ASCII character!)
 */
unsigned char read_scan_code(void);

void keyboard_init(void);

#endif