#include "io.h"

/* The Colors usable by text */
#define COLOR_BLACK         0
#define COLOR_BLUE          1
#define COLOR_GREEN         2
#define COLOR_CYAN          3
#define COLOR_RED           4
#define COLOR_MAGENTA       5
#define COLOR_BROWN         6
#define COLOR_LIGHT_GREY    7
#define COLOR_DARK_GREY     8
#define COLOR_LIGHT_BLUE    9
#define COLOR_LIGHT_GREEN   10
#define COLOR_LIGHT_CYAN    11
#define COLOR_LIGHT_RED     12
#define COLOR_LIGHT_MAGENTA 13
#define COLOR_LIGHT_BROWN   14
#define COLOR_WHITE         15

/* The I/O ports */
#define FB_COMMAND_PORT         0x3D4
#define FB_DATA_PORT            0x3D5
#define SERIAL_COM1_BASE        0x3F8      /* COM1 base port */

#define SERIAL_DATA_PORT(base)          (base)
#define SERIAL_FIFO_COMMAND_PORT(base)  (base + 2)
#define SERIAL_LINE_COMMAND_PORT(base)  (base + 3)
#define SERIAL_MODEM_COMMAND_PORT(base) (base + 4)
#define SERIAL_LINE_STATUS_PORT(base)   (base + 5)

/* The I/O Port Commands */
#define FB_HIGH_BYTE_COMMAND    14
#define FB_LOW_BYTE_COMMAND     15

/* SERIAL_LINE_ENABLE_DLAB:
* Tells the serial port to expect first the highest 8 bits on the data port,
* then the lowest 8 bits will follow
*/
#define SERIAL_LINE_ENABLE_DLAB         0x80

char *fb = (char *) 0x000B8000;

/* Write a text on screen */
void fb_write_cell(unsigned int i, char c, unsigned char bg, unsigned char fg) {
	fb[i] = c;
	fb[i + 1] = ((bg & 0x0F) << 4 | (fg & 0x0F));
}

/** serial_configure_baud_rate:
 *  Sets the speed of the data being sent. The default speed of a serial
 *  port is 115200 bits/s. The argument is a divisor of that number, hence
 *  the resulting speed becomes (115200 / divisor) bits/s.
 *
 *  @param com      The COM port to configure
 *  @param divisor  The divisor
 */
void serial_configure_baud_rate(unsigned short com, unsigned short divisor) {
	outb(SERIAL_LINE_COMMAND_PORT(com), SERIAL_LINE_ENABLE_DLAB);
	outb(SERIAL_DATA_PORT(com), 		(divisor >> 8) & 0x00FF);
	outb(SERIAL_DATA_PORT(com), 		divisor & 0x00FF);
}

/** serial_configure_line:
 *  Configures the line of the given serial port. The port is set to have a
 *  data length of 8 bits, no parity bits, one stop bit and break control
 *  disabled.
 *
 *  @param com  The serial port to configure
 */
void serial_configure_line(unsigned short com)
{
	/* Bit:    | 7 | 6 | 5 4 3 | 2 | 1 0 |
	* Content: | d | b | prty  | s | dl  |
	* Value:   | 0 | 0 | 0 0 0 | 0 | 1 1 | = 0x03
	*/
	outb(SERIAL_LINE_COMMAND_PORT(com), 0x03);
}

/** fb_move_cursor:
 *  Moves the cursor of the framebuffer to the given position
 *
 *  @param pos The new position of the cursor
 */
void fb_move_cursor(unsigned short pos)
{
	outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
	outb(FB_DATA_PORT,    ((pos >> 8) & 0x00FF));
	outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
	outb(FB_DATA_PORT,    pos & 0x00FF);
}

void kmain() {
	fb_write_cell(0, 'A', COLOR_LIGHT_BLUE, COLOR_WHITE);
	fb_move_cursor(0);
}