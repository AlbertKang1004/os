#include "io.h"
#include "gdt.h"
#include "interrupt.h"
#include "interrupt_handlers.h"
#include "pic.h"
#include "keyboard.h"

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

unsigned long long gdt[3] = {
	0x0000000000000000, 
	0x00CF9A000000FFFF,
    0x00CF92000000FFFF
};

struct idt_entry idt[256];

char *fb = (char *) 0x000B8000;

/* =============== SERIAL PORT ================ */

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

/** serial_is_transmit_fifo_empty:
 *  Checks whether the transmit FIFO queue is empty or not for the given COM
 *  port.
 *
 *  @param  com The COM port
 *  @return 0 if the transmit FIFO queue is not empty
 *          1 if the transmit FIFO queue is empty
 */
int serial_is_transmit_fifo_empty(unsigned int com)
{
	/* 0x20 = 0010 0000 */
	return inb(SERIAL_LINE_STATUS_PORT(com)) & 0x20;
}

/** serial_write:
 *  Writes a text to the serial port.
 * 
 *  @param  com The COM port
 * 	@param  text The text to be written to the port
 */
void serial_write(unsigned int com, char* text) {
	while (*text != 0) {
		while (!serial_is_transmit_fifo_empty(com));
		// wait until buffer is empty
		outb(SERIAL_DATA_PORT(com), *text++);
	}
}

/* =============== FRAMEBUFFER ================ */

/** fb_write_cell:
 *  Writes a character with colors to a cell in the framebuffer
 *
 *  @param i    The index of the cell
 *  @param c    The character to write
 *  @param bg   The background color
 *  @param fg   The foreground color
 */
void fb_write_cell(unsigned int i, char c, unsigned char bg, unsigned char fg) {
	fb[i] = c;
	fb[i + 1] = ((bg & 0x0F) << 4 | (fg & 0x0F));
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

/** fb_write:
 *  Writes a text to the framebuffer.
 * 	@param  text 	The text to be written 
 *  @param  bg   	Background Color
 *  @param  fg   	Text Color
 *  @param  offset  The offset for the text to be written
 *  @return  0 if successfully written
 * 			 1 if not
 */
int fb_write(char* text, unsigned char bg, unsigned char fg, unsigned int offset) {
	while (*text != 0) {
		fb[offset++] = *text++;
		fb[offset++] = ((bg & 0x0F) << 4 | (fg & 0x0F));
	}
	return 0;
}

/** print_hex:
 *  Converts an unsigned integer to a hexadecimal string
 *
 *  @param n    The unsigned integer to convert
 *  @return     The hexadecimal string representation of n
 */
char *print_hex(unsigned int n) {
    static char hex[] = "0x00000000";
    int i;
    for (i = 9; i >= 2; i--) {
        int digit = n & 0xF;
        hex[i] = digit < 10 ? '0' + digit : 'A' + digit - 10;
        n >>= 4;
    }
    return hex;
}

/* ================ INTERRUPT ================= */

/** interrupt_handler:
 *  Handles the interrupt by delegating to the appropriate handler
 * 
 *  @param cpu          The CPU register state at the time of the interrupt
 *  @param stack        The stack state pushed by the CPU when the interrupt occurred
 *  @param interrupt    The interrupt number
 */
void interrupt_handler(struct cpu_state *cpu, struct stack_state *stack, unsigned int interrupt) {
	(void) cpu;
	if (interrupt == 0x21) {
		unsigned char scan_code = read_scan_code();
		serial_write(SERIAL_COM1_BASE, "Key: ");
		serial_write(SERIAL_COM1_BASE, print_hex(scan_code));
		pic_acknowledge(interrupt);
	} else {
		serial_write(SERIAL_COM1_BASE, "Interrupt: ");
		serial_write(SERIAL_COM1_BASE, print_hex(interrupt));
		serial_write(SERIAL_COM1_BASE, " Code: ");
		serial_write(SERIAL_COM1_BASE, print_hex(stack->error_code));
	}
	serial_write(SERIAL_COM1_BASE, "\n");
}

/** create_idt_entry:
 *  Creates an entry in the IDT for the given interrupt handler
 *
 *  @param index        The index of the interrupt handler in the IDT
 *  @param handler      The address of the interrupt handler
 */
void create_idt_entry(unsigned int index, unsigned int handler) {
	struct idt_entry idt_e;
	idt_e.offset_high = (handler >> 16) & 0xFFFF;
	idt_e.offset_low = handler & 0xFFFF;
	idt_e.reserved = 0;
	idt_e.segment = 0x0008;
	idt_e.type_attr = 0x8E;
	// 1110 1000
	idt[index] = idt_e;
}

/* ================= UTILITY ================== */

void kmain() {
	struct gdt_descriptor gdt_desc;
	gdt_desc.size = sizeof(gdt) - 1;
	gdt_desc.address = (unsigned int)(unsigned long)  gdt;
	gdt_flush(&gdt_desc);

	fb_move_cursor(0);

	serial_configure_baud_rate(SERIAL_COM1_BASE, 1);
	serial_configure_line(SERIAL_COM1_BASE);

	serial_write(SERIAL_COM1_BASE, "Hello World!");
	fb_write("Hello People and Computers.", COLOR_CYAN, COLOR_BLACK, 0);
	
	for (int i = 0; i < 256; i++) {
		create_idt_entry(i, (unsigned int)(unsigned long) interrupt_handlers[i]);
	} 

	struct idt_descriptor idt_desc;
	idt_desc.size = sizeof(idt);
	idt_desc.address = (unsigned int)(unsigned long) idt;
	load_idt(&idt_desc);

	pic_remap(0x20, 0x28);
	outb(0x21, 0x01);
	__asm__("sti"); // activate PIC Interrupts
}
