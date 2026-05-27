#include "../lib/io.h"
#include "gdt.h"
#include "interrupt.h"
#include "interrupt_handlers.h"
#include "../drivers/pic.h"
#include "../drivers/keyboard.h"
#include "../drivers/serial.h"
#include "debug.h"
#include "multiboot.h"
#include "pmm.h"
#include "utils.h"

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

extern unsigned int multiboot_mods_addr;
extern unsigned int multiboot_mmap_addr;
extern unsigned int multiboot_mmap_length;
extern unsigned int page_directory[];
extern unsigned int page_table[];
extern unsigned int kernel_virtual_start;
extern unsigned int kernel_virtual_end;
extern unsigned int kernel_physical_start;
extern unsigned int kernel_physical_end;

typedef void (*call_module_t) (void);

unsigned long long gdt[3] = {
	0x0000000000000000, 
	0x00CF9A000000FFFF,
    0x00CF92000000FFFF
};

struct idt_entry idt[256];

char *fb = (char *) 0xC00B8000;

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
        // Keyboard interrupt
        unsigned char scan_code = read_scan_code();
        LOG_HEX("Key", scan_code);
        pic_acknowledge(interrupt);
    } else {
        // Other interrupts
        LOG_HEX("Interrupt", interrupt);
        LOG_HEX("Code", stack->error_code);
    }
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

void kmain() {
    // GDT setup
    struct gdt_descriptor gdt_desc;
    gdt_desc.size = sizeof(gdt) - 1;
    gdt_desc.address = (unsigned int)(unsigned long) gdt;
    gdt_flush(&gdt_desc);

    // Serial / framebuffer init
    fb_move_cursor(0);
    serial_configure_baud_rate(SERIAL_COM1_BASE, 1);
    serial_configure_line(SERIAL_COM1_BASE);
    LOG_HEX("multiboot addr", multiboot_mods_addr);
    LOG("Hello World!");
    fb_write("Hello People and Computers.\n", COLOR_CYAN, COLOR_BLACK, 0);

    // IDT setup
    for (int i = 0; i < 256; i++) {
        create_idt_entry(i, (unsigned int)(unsigned long) interrupt_handlers[i]);
    }
    struct idt_descriptor idt_desc;
    idt_desc.size = sizeof(idt);
    idt_desc.address = (unsigned int)(unsigned long) idt;
    load_idt(&idt_desc);

    // PIC setup and enable interrupts
    pic_remap(0x20, 0x28);
    outb(0x21, 0x01);
    __asm__("sti");

    // Kernel virtual/physical address labels
    unsigned int virt_start  = (unsigned int)(unsigned long) &kernel_virtual_start;
    unsigned int virt_end    = (unsigned int)(unsigned long) &kernel_virtual_end;
    unsigned int phys_start  = (unsigned int)(unsigned long) &kernel_physical_start;
    unsigned int phys_end    = (unsigned int)(unsigned long) &kernel_physical_end;

    // Multiboot module
    multiboot_module_t *module = (multiboot_module_t *)(unsigned long) (multiboot_mods_addr + KERNEL_VIRTUAL_BASE);

    // Physical memory manager init
    LOG_HEX("mmap_addr", multiboot_mmap_addr);
    LOG_HEX("mmap_length", multiboot_mmap_length);
    pmm_init(multiboot_mmap_addr + 0xC0000000, multiboot_mmap_length);

    // Dump mmap entries
    struct mmap_entry *e = (struct mmap_entry *)(unsigned long)(multiboot_mmap_addr + 0xC0000000);
    while ((unsigned int)e < multiboot_mmap_addr + 0xC0000000 + multiboot_mmap_length) {
        LOG_HEX("addr", e->addr_low);
        LOG_HEX("len", e->len_low);
        LOG_HEX("type", e->type);
        e = (struct mmap_entry *)((unsigned int)e + e->size + 4);
    }

    // Verify kernel pages are marked as used
    LOG_HEX("kernel phys_start", phys_start);
    LOG_HEX("kernel phys_end", phys_end);
    unsigned int start_page = phys_start / 0x1000;
    LOG_HEX("start_page", start_page);
    LOG_HEX("bitmap at kernel", page_bitmap[start_page / 32]);

    // Test pmm_alloc / pmm_free
    unsigned int frame = pmm_alloc();
    LOG_HEX("allocated", frame);
    pmm_free(frame);
    unsigned int frame2 = pmm_alloc();
    LOG_HEX("allocated after free", frame2);

    // Jump to user module
    call_module_t start_program = (call_module_t)(unsigned long) module->mod_start + KERNEL_VIRTUAL_BASE;
    start_program();
}