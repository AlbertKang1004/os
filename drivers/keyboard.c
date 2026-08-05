#include "../lib/io.h"
#include "../kernel/debug.h"
#include "../kernel/interrupt.h"
#include "../kernel/scheduler.h"
#include "keyboard.h"
#include "pic.h"

static int saw_e0; // 1 if true, 0 if false
static unsigned char mods;
static unsigned char buf[128];
static unsigned int read_pos = 0;
static unsigned int write_pos = 0;

static inline void set_mod(unsigned char key, int released) {
    mods = released ? mods & ~key : mods | key;
}

/* Scancode set 1 -> ASCII. Index is the make code (bit 7 clear).
 * Unlisted entries stay 0, meaning "this key produces no character". */
static const unsigned char map[256] = {
    [0x02] = '1',  [0x03] = '2',  [0x04] = '3',  [0x05] = '4',  [0x06] = '5',
    [0x07] = '6',  [0x08] = '7',  [0x09] = '8',  [0x0A] = '9',  [0x0B] = '0',
    [0x0C] = '-',  [0x0D] = '=',  [0x0E] = '\b', [0x0F] = '\t',

    [0x10] = 'q',  [0x11] = 'w',  [0x12] = 'e',  [0x13] = 'r',  [0x14] = 't',
    [0x15] = 'y',  [0x16] = 'u',  [0x17] = 'i',  [0x18] = 'o',  [0x19] = 'p',
    [0x1A] = '[',  [0x1B] = ']',  [0x1C] = '\n',

    [0x1E] = 'a',  [0x1F] = 's',  [0x20] = 'd',  [0x21] = 'f',  [0x22] = 'g',
    [0x23] = 'h',  [0x24] = 'j',  [0x25] = 'k',  [0x26] = 'l',  [0x27] = ';',
    [0x28] = '\'', [0x29] = '`',  [0x2B] = '\\',

    [0x2C] = 'z',  [0x2D] = 'x',  [0x2E] = 'c',  [0x2F] = 'v',  [0x30] = 'b',
    [0x31] = 'n',  [0x32] = 'm',  [0x33] = ',',  [0x34] = '.',  [0x35] = '/',

    [0x39] = ' ',
};

static const unsigned char shift_map[256] = {
    [0x02] = '!',  [0x03] = '@',  [0x04] = '#',  [0x05] = '$',  [0x06] = '%',
    [0x07] = '^',  [0x08] = '&',  [0x09] = '*',  [0x0A] = '(',  [0x0B] = ')',
    [0x0C] = '_',  [0x0D] = '+',  [0x0E] = '\b', [0x0F] = '\t',

    [0x10] = 'Q',  [0x11] = 'W',  [0x12] = 'E',  [0x13] = 'R',  [0x14] = 'T',
    [0x15] = 'Y',  [0x16] = 'U',  [0x17] = 'I',  [0x18] = 'O',  [0x19] = 'P',
    [0x1A] = '{',  [0x1B] = '}',  [0x1C] = '\n',

    [0x1E] = 'A',  [0x1F] = 'S',  [0x20] = 'D',  [0x21] = 'F',  [0x22] = 'G',
    [0x23] = 'H',  [0x24] = 'J',  [0x25] = 'K',  [0x26] = 'L',  [0x27] = ':',
    [0x28] = '"',  [0x29] = '~',  [0x2B] = '|',

    [0x2C] = 'Z',  [0x2D] = 'X',  [0x2E] = 'C',  [0x2F] = 'V',  [0x30] = 'B',
    [0x31] = 'N',  [0x32] = 'M',  [0x33] = '<',  [0x34] = '>',  [0x35] = '?',

    [0x39] = ' ',
};

/** keyboard_write:
 *  Appends one character to the keyboard input buffer. This is the
 *  producer side of the queue, called from the IRQ1 handler once the
 *  decoder has turned a scan code into an actual character.
 *
 *  Runs in interrupt context, so it must not block or allocate: the
 *  buffer is a fixed-size array and the failure mode is simply to drop
 *  the character.
 *
 *  When the buffer is full the NEW character is dropped rather than
 *  overwriting the oldest one. Characters already queued are input the
 *  user definitely typed and the reader is about to consume; discarding
 *  those would silently truncate a command line, whereas dropping the
 *  newest is visible to the user, who can simply type it again.
 *
 *  @param c  The character to enqueue.
 *  @return   0 if the character was stored, -1 if the buffer was full
 *            and the character was dropped.
 */
static int keyboard_write(unsigned char c) {
    if (write_pos - read_pos == 128) // when the queue is full
        return -1;
    buf[write_pos++ & 127] = c;
    scheduler_wake_blocked();
    return 0;
}

/** keyboard_read:
 *  Removes and returns the oldest character from the keyboard input
 *  buffer. Characters are produced by the IRQ1 handler, which decodes
 *  scan codes and enqueues the resulting ASCII; this is the consumer
 *  side of that queue.
 *
 *  Non-blocking: returns immediately whether or not input is available.
 *  The caller decides what to do when the buffer is empty.
 *
 *  The return value must be stored in an int, not a char -- a char
 *  cannot hold -1 distinctly from a valid character on all platforms.
 *
 *  @return The next character (0-255), or -1 if the buffer is empty.
 */
int keyboard_read() {
    if (read_pos == write_pos) return -1; // nothing to read
    unsigned char c = buf[read_pos++ & 127];
    return c;
}

static int scan_code_decoder(unsigned char scan_code) {
    if (scan_code == 0xE0) { // extended prefix
        saw_e0 = 1; // true
        return -1;
    } 
    int released = scan_code & 0x80;
    int ext = saw_e0;
    saw_e0 = 0;
    scan_code &= 0x7F;
    switch (scan_code) {
        case 0x2A: // LShift
            if (!ext) set_mod(MOD_LSHIFT, released);
            break;
        case 0x36: // RShift
            if (!ext) set_mod(MOD_RSHIFT, released);
            break;
        case 0x1D: // LCtrl / RCtrl
            set_mod(ext ? MOD_RCTRL : MOD_LCTRL, released);
            break;
        case 0x38:
            set_mod(ext ? MOD_RALT : MOD_LALT, released);
            break;
        case 0x3A: // Caps
            if (released) return -1;
            mods ^= MOD_CAPS; // toggle
            break;
        default: {
            if (released || ext) return -1;
            
            int shift = (mods & (MOD_LSHIFT|MOD_RSHIFT)) != 0;
            unsigned char c = map[scan_code];
            if ((c >= 'a' && c <= 'z') && (mods & MOD_CAPS)) shift = !shift;
            c = (shift ? shift_map[scan_code] : map[scan_code]);
            return c ? c : -1;
        }
    }
    return -1;
}

/** keyboard_interrupt_handler:
 *  IRQ1 handler, registered with the interrupt table by keyboard_init.
 *  Fires once per byte the PS/2 controller delivers -- note that a
 *  single key press can produce several bytes and therefore several
 *  interrupts (extended keys are prefixed with 0xE0).
 *
 *  Reads exactly one byte from the data port, feeds it to the decoder,
 *  and enqueues the result if a character was produced. Most bytes
 *  produce none: key releases, modifiers, and prefixes only update the
 *  decoder's internal state.
 *
 *  The port read is mandatory even for bytes that are ignored -- the
 *  controller's output buffer holds one byte, and it will not deliver
 *  another until this one is consumed. The PIC acknowledgement is
 *  mandatory for the same reason: without it no further IRQ1 arrives.
 *
 *  No locking is needed against the consumer side (keyboard_read):
 *  interrupt gates clear IF, so this handler cannot run while a syscall
 *  is midway through updating the queue indices.
 *
 *  @param cpu        Saved registers of the interrupted context. Unused.
 *  @param stack      Saved iret frame. Unused.
 *  @param interrupt  The interrupt number, passed on to the PIC as the
 *                    IRQ to acknowledge.
 */
static void keyboard_interrupt_handler(struct cpu_state * cpu, struct stack_state * stack, unsigned int interrupt) {
    (void) cpu;
    (void) stack;
    // Keyboard interrupt
    int decoded_char = scan_code_decoder(read_scan_code());
    if (decoded_char != -1) 
        keyboard_write(decoded_char);
    pic_acknowledge(interrupt);
}

/** read_scan_code:
 *  Reads a scan code from the keyboard
 *
 *  @return The scan code (NOT an ASCII character!)
 */
unsigned char read_scan_code(void) {
    return inb(KBD_DATA_PORT);
}

void keyboard_init() {
    register_interrupt_handler(0x21, keyboard_interrupt_handler);
}