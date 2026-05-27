#include "pmm.h"
#include "../drivers/serial.h"
#include "utils.h"

unsigned int page_bitmap = 0xFFFFFFFF;

void kernel_physical_start(void);
void kernel_physical_end(void);

/** pmm_init:
 *  Initializes the physical memory manager bitmap
 */
void pmm_init(unsigned int map_addr, unsigned int map_length) {
    struct mmap_entry * mmap = (struct mmap_entry *)(unsigned long) map_addr;
    while ((unsigned int)mmap < map_addr + map_length) {
        serial_write(SERIAL_COM1_BASE, "page: ");
        serial_write(SERIAL_COM1_BASE, print_hex(mmap->addr_low / PAGE_SIZE));
        serial_write(SERIAL_COM1_BASE, " count: ");
        serial_write(SERIAL_COM1_BASE, print_hex(mmap->len_low / PAGE_SIZE));
        serial_write(SERIAL_COM1_BASE, " type: ");
        serial_write(SERIAL_COM1_BASE, print_hex(mmap->type));
        serial_write(SERIAL_COM1_BASE, "\n");
        if (mmap -> type == 1) {
            unsigned int page = mmap->addr_low / PAGE_SIZE;
            unsigned int count = mmap->len_low / PAGE_SIZE;
            for (unsigned int i = page; i < page + count && i < MAX_PAGES; i++) {
                page_bitmap &= ~(1 << i);
            }
        } 
        mmap = (struct mmap_entry *)((unsigned int)mmap + mmap->size + 4);     
    }
    // mark kernel pages as used
    unsigned int phys_start = (unsigned int) &kernel_physical_start;
    unsigned int phys_end   = (unsigned int) &kernel_physical_end;
    unsigned int kernel_start_page = phys_start / PAGE_SIZE;
    unsigned int kernel_end_page = phys_end / PAGE_SIZE;
    for (unsigned int i = kernel_start_page; i <= kernel_end_page; i++) {
        page_bitmap |= (1 << i);
    }
}

/** pmm_alloc:
 *  Finds a free page in the bitmap, marks it as used and returns its physical address
 *
 *  @return     The physical address of the allocated page
 */
unsigned int pmm_alloc() {

}

/** pmm_free:
 *  Marks a page as free in the bitmap
 *
 *  @param address  The physical address of the page to free
 */
void pmm_free(unsigned int address) {

}
