#include "fb.h"
#include "../lib/io.h"

/* There are two cursors. `cursor` below is this driver's own bookkeeping
 * (where the next character goes); the hardware cursor is the blinking
 * block on screen and only moves when fb_move_cursor writes the CRT
 * registers. Nothing keeps them in sync automatically, so the rule is:
 * public functions leave both in agreement on return, private helpers do
 * not have to. That lets a whole string be drawn with a single (slow)
 * port update at the end.
 *
 * Layering: fb_write_cell/fb_fill draw at absolute positions and never
 * touch `cursor` -- that is what lets fb_clear and fb_scroll reuse them.
 * fb_putchar owns the cursor and interprets control characters. */

static unsigned int cursor = 0;
// static unsigned char bg_color = COLOR_BLACK;
// static unsigned char fg_color = COLOR_WHITE;
static unsigned char attr = 0x07; // black background, light gray text
static unsigned short *fb = (unsigned short *) 0xC00B8000;

/** fb_write_cell:
 *  Draws one character at an absolute cell, using the current attribute.
 *  The cast to unsigned char matters: a signed char with the high bit set
 *  would sign extend and flood the attribute byte with ones.
 *
 *  @param i    The index of the cell
 *  @param c    The character to write
 */
static void fb_write_cell(unsigned int i, char c) {
	fb[i] = attr << 8 | (unsigned char) c;
}

/** fb_fill:
 *  Writes the same character over a run of cells. Blanking a region means
 *  filling it with spaces rather than zeroes, so every cell on screen
 *  always carries a valid attribute.
 *
 *  @param start    First cell of the run
 *  @param count    How many cells to write
 *  @param cell     The character to repeat
 */
static void fb_fill(unsigned int start, unsigned int count, char cell) {
	for (unsigned int i = 0; i < count; i++) {
		fb_write_cell(start + i, cell);
	}
}

/** fb_scroll:
 *  Moves every line up by one, blanks the freed bottom line and pulls the
 *  cursor back with it. The copy runs forwards because each destination
 *  cell is read before it is written -- the source is always one row
 *  ahead of the destination.
 */
static void fb_scroll(void) {
	for (int i = 0; i < FB_CELLS - FB_WIDTH; i++) {
		fb[i] = fb[i + FB_WIDTH];
	}
	fb_fill(FB_CELLS - FB_WIDTH, FB_WIDTH, ' '); // make last line empty
	cursor -= FB_WIDTH;
}

/** fb_move_cursor:
 *  Moves the blinking hardware cursor. The position is a cell index and
 *  does not fit in one register, so it goes out as two halves. This is
 *  four port writes -- expensive next to a memory store, which is why it
 *  is called once per public call and not once per character.
 *
 *  @param pos The new position of the cursor
 */
static void fb_move_cursor(unsigned short pos) {
	outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
	outb(FB_DATA_PORT,    ((pos >> 8) & 0x00FF));
	outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
	outb(FB_DATA_PORT,    pos & 0x00FF);
}

/** fb_putchar:
 *  Advances the console by one character. Control characters only move
 *  the cursor: \b steps back without erasing, so a caller that wants to
 *  rub a character out sends "\b \b" -- the same three bytes a serial
 *  terminal needs, which keeps both devices in step.
 *
 *  The overflow check sits at the end rather than in the default case
 *  because a newline on the last line overflows too.
 *
 *  @param c    The character to print
 */
static void fb_putchar(char c) {
	switch (c) {
		case '\n':
			cursor = (cursor / FB_WIDTH + 1) * FB_WIDTH;
			break;
		case '\r':
			cursor = (cursor / FB_WIDTH) * FB_WIDTH;
			break;
		case '\b':
			if (cursor > 0) cursor--;
			break;
		case '\t':
			cursor = (cursor / FB_TAB + 1) * FB_TAB;
			break;
		default:
			fb_write_cell(cursor, c);
			cursor++;
			break;
	}
	if (cursor >= FB_CELLS) fb_scroll();
}

/** fb_clear:
 *  Blanks the screen and returns both cursors to the top left. Needed at
 *  boot because GRUB leaves its own output behind.
 */
void fb_clear(void) {
	fb_fill(0, FB_CELLS, ' ');
	cursor = 0;
	fb_move_cursor(cursor);
}

/** fb_write:
 *  Prints a buffer to the console. The length is explicit because this is
 *  fed from sys_write's user buffer, which carries no terminator. Syncs
 *  the hardware cursor once, after the last character.
 *
 *  @param buf  The bytes to print (not NUL terminated)
 *  @param len  How many bytes to print
 *  @return     Bytes written
 */
int fb_write(const char * buf, unsigned int len) {
	const char * cur = buf;
	for (unsigned int i = 0; i < len; i++) {
		fb_putchar(*cur++);
	}
	
	fb_move_cursor(cursor);
	return len;
}
