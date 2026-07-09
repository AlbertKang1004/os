#include "../lib/io.h"
#include "exceptions.h"
#include "gdt.h"
#include "interrupt.h"
#include "interrupt_handlers.h"
#include "../drivers/pic.h"
#include "../drivers/pit.h"
#include "../drivers/keyboard.h"
#include "../drivers/serial.h"
#include "cpu.h"
#include "debug.h"
#include "interrupt.h"
#include "kmalloc.h"
#include "multiboot.h"
#include "pmm.h"
#include "process.h"
#include "scheduler.h"
#include "syscall.h"
#include "tss.h"
#include "utils.h"
#include "vmm.h"

extern unsigned int multiboot_info_ptr;
extern unsigned int kernel_virtual_start;
extern unsigned int kernel_virtual_end;
extern unsigned int kernel_physical_start;
extern unsigned int kernel_physical_end;
extern unsigned int kernel_stack_top;
extern unsigned int gdt_size;
extern struct gdt_entry gdt[];
extern void enter_user_mode(unsigned int entry, unsigned int user_stack);

typedef void (*call_module_t) (void);
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
    idt_e.segment = 0x0008; // kernel code segment
    idt_e.type_attr = (index == 0x80) ? 0xEE : 0x8E; 
	idt[index] = idt_e;
}

void kmain() {
    // GDT setup
    // Index 	Offset 	Name 	            Address range 	            Type 	DPL
    // 0 	    0x00 	null descriptor 			
    // 1 	    0x08 	kernel code segment 0x00000000 - 0xFFFFFFFF 	RX 	    PL0
    // 2 	    0x10 	kernel data segment 0x00000000 - 0xFFFFFFFF 	RW 	    PL0
    // 3 	    0x18 	user code segment 	0x00000000 - 0xFFFFFFFF 	RX 	    PL3
    // 4 	    0x20 	user data segment 	0x00000000 - 0xFFFFFFFF 	RW 	    PL3

    gdt_set_entry(0, 0, 0,          0,    0);       // null
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);    // kernel code PL0
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);    // kernel data PL0
    gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);    // user code PL3
    gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);    // user data PL3
    gdt_set_entry(5, (unsigned int) &tss, sizeof(tss) - 1, 0x89, 0x00);    // TSS selector

    struct gdt_descriptor gdt_desc;
    gdt_desc.size = gdt_size - 1;
    gdt_desc.address = (unsigned int)(unsigned long) gdt;
    gdt_flush(&gdt_desc);

    // TSS setup
    tss_init((unsigned int) &kernel_stack_top);

    // Serial / framebuffer init
    fb_move_cursor(0);
    serial_configure_baud_rate(SERIAL_COM1_BASE, 1);
    serial_configure_line(SERIAL_COM1_BASE);
    fb_write("Hello People and Computers.\n", COLOR_CYAN, COLOR_BLACK, 0);

    // IDT setup
    for (int i = 0; i < 256; i++) {
        create_idt_entry(i, (unsigned int)(unsigned long) interrupt_handlers[i]);
    }
    struct idt_descriptor idt_desc;
    idt_desc.size = sizeof(idt);
    idt_desc.address = (unsigned int)(unsigned long) idt;
    load_idt(&idt_desc);

    keyboard_init();
    syscall_init();
    page_fault_init();

    // PIC setup and enable interrupts + PIT setup
    pic_remap(0x20, 0x28);
    pit_init(100);
    outb(0x21, 0x00);
    __asm__("sti");

    multiboot_info_t *mb = (multiboot_info_t *)(unsigned long)(multiboot_info_ptr + KERNEL_VIRTUAL_BASE);
    // Physical memory manager init
    pmm_init(mb->mmap_addr + 0xC0000000, mb->mmap_length);
    pmm_reserve_region(mb->mods_addr, mb->mods_addr + (mb->mods_count * sizeof(multiboot_module_t)));
    pmm_reserve_region(multiboot_info_ptr, multiboot_info_ptr + sizeof(multiboot_info_t));
    pmm_reserve_region(mb->mmap_addr, mb->mmap_addr + mb->mmap_length);

    // testing goes here...

    // Jump to user module
    multiboot_module_t *module = (multiboot_module_t *)(unsigned long) (mb->mods_addr + KERNEL_VIRTUAL_BASE);
    for (unsigned int i = 0; i < mb->mods_count; i++) {
        pmm_reserve_region(module[i].mod_start, module[i].mod_end);
    }
    
    unsigned int phys_end = (unsigned int)(unsigned long) &kernel_physical_end;

    struct process * proc = process_create(module[0].mod_start, module[0].mod_end - module[0].mod_start);
    struct process * proc2 = process_create(module[1].mod_start, module[1].mod_end - module[1].mod_start);   
    scheduler_add(proc);
    scheduler_add(proc2);

    unsigned int entry  = proc->code_addr;
    unsigned int ustack = proc->stack_addr;
    unsigned int pd     = proc->page_directory;
    write_cr3(pd);

    enter_user_mode(entry, ustack);
}

/* DEBUGGING PMM
    unsigned int virt_start  = (unsigned int)(unsigned long) &kernel_virtual_start;
    unsigned int virt_end    = (unsigned int)(unsigned long) &kernel_virtual_end;
    unsigned int phys_start  = (unsigned int)(unsigned long) &kernel_physical_start;
    unsigned int phys_end    = (unsigned int)(unsigned long) &kernel_physical_end;
    LOG_HEX("mmap_addr", multiboot_mmap_addr);
    LOG_HEX("mmap_length", multiboot_mmap_length);
    struct mmap_entry *e = (struct mmap_entry *)(unsigned long)(multiboot_mmap_addr + 0xC0000000);
    while ((unsigned int)e < multiboot_mmap_addr + 0xC0000000 + multiboot_mmap_length) {
        LOG_HEX("addr", e->addr_low);
        LOG_HEX("len", e->len_low);
        LOG_HEX("type", e->type);
        e = (struct mmap_entry *)((unsigned int)e + e->size + 4);
    }
    LOG_HEX("kernel phys_start", phys_start);
    LOG_HEX("kernel phys_end", phys_end);
    unsigned int start_page = phys_start / 0x1000;
    LOG_HEX("start_page", start_page);
    LOG_HEX("bitmap at kernel", page_bitmap[start_page / 32]);
    unsigned int frame = pmm_alloc();
    LOG_HEX("allocated", frame);
    pmm_free(frame);
    unsigned int frame2 = pmm_alloc();
    LOG_HEX("allocated after free", frame2);
 */

 /* DEBUGGING VMM
    unsigned int phys = pmm_alloc();
    LOG_HEX("phys allocated", phys);

    vmm_map_page(phys, 0xD0000000, PAGE_PRESENT | PAGE_RW);

    unsigned int *test = (unsigned int *)0xD0000000;
    *test = 0xDEADBEEF;
    LOG_HEX("value at 0xD0000000", *test);

    unsigned int resolved = vmm_get_phys(0xD0000000);
    LOG_HEX("resolved phys", resolved);

    vmm_unmap_page(0xD0000000);
    unsigned int after = vmm_get_phys(0xD0000000);
    LOG_HEX("after unmap", after);
 */

 /* DEBUGGING MEMORY ALLOCATION

    // Test 1: basic allocation
    unsigned int *a = (unsigned int *)kmalloc(sizeof(unsigned int) * 4);
    if (a == 0) {
        serial_write(SERIAL_COM1_BASE, "kmalloc a failed\n");
    } else {
        a[0] = 1; a[1] = 2; a[2] = 3; a[3] = 4;
        LOG_HEX("a[0]", a[0]);
        LOG_HEX("a[1]", a[1]);
        LOG_HEX("a[2]", a[2]);
        LOG_HEX("a[3]", a[3]);
    }

    // Test 2: second allocation should be at different address
    unsigned int *b = (unsigned int *)kmalloc(sizeof(unsigned int) * 4);
    if (b == 0) {
        serial_write(SERIAL_COM1_BASE, "kmalloc b failed\n");
    } else {
        b[0] = 10; b[1] = 20;
        LOG_HEX("b[0]", b[0]);
        LOG_HEX("b[1]", b[1]);
    }

    // Test 3: free a and reallocate, should reuse same address
    kfree(a);
    unsigned int *c = (unsigned int *)kmalloc(sizeof(unsigned int) * 4);
    LOG_HEX("a addr", (unsigned int)(unsigned long)a);
    LOG_HEX("c addr", (unsigned int)(unsigned long)c);
    // if a == c, kfree and reuse working correctly
 */
