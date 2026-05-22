#include "../lib/io.h"
#include "keyboard.h"

/** read_scan_code:
 *  Reads a scan code from the keyboard
 *
 *  @return The scan code (NOT an ASCII character!)
 */
unsigned char read_scan_code(void) {
    return inb(KBD_DATA_PORT);
}