#include "pmm.h"

unsigned int page_bitmap[1024];

void kernel_physical_start(void);
void kernel_physical_end(void);

static void bitmap_set(unsigned int page) {
    page_bitmap[page / 32] |= (1 << (page % 32));
}

static void bitmap_clear(unsigned int page) {
    page_bitmap[page / 32] &= ~(1 << (page % 32));
}

static unsigned int bitmap_test(unsigned int page) {
    return page_bitmap[page / 32] & (1 << (page % 32));
}

/**
 * pmm_init:
 * Initializes the physical memory manager bitmap
 *
 * @param map_addr    Physical address of the BIOS/bootloader-provided memory map
 * @param map_length  Total byte length of the memory map structure
 */
void pmm_init(unsigned int map_addr, unsigned int map_length) {
    for (unsigned int i = 0; i < 1024; i++)
        page_bitmap[i] = 0xFFFFFFFF;

    struct mmap_entry * mmap = (struct mmap_entry *)(unsigned long) map_addr;
    while ((unsigned int)(unsigned long) mmap < map_addr + map_length) {
        if (mmap -> type == 1) {
            unsigned int page = mmap->addr_low / PAGE_SIZE;
            unsigned int count = mmap->len_low / PAGE_SIZE;
            for (unsigned int i = page; i < page + count && i < MAX_PAGES; i++) {
                bitmap_clear(i);
            }
        } 
        mmap = (struct mmap_entry *)((unsigned int)mmap + mmap->size + 4);     
    }
    // mark kernel pages as used
    unsigned int phys_start = (unsigned int)(unsigned long) &kernel_physical_start;
    unsigned int phys_end   = (unsigned int)(unsigned long) &kernel_physical_end;
    unsigned int kernel_start_page = phys_start / PAGE_SIZE;
    unsigned int kernel_end_page = phys_end / PAGE_SIZE;
    for (unsigned int i = kernel_start_page; i <= kernel_end_page; i++) {
        bitmap_set(i);
    }
}

/** pmm_alloc:
 *  Finds a free page in the bitmap, marks it as used and returns its physical address
 *
 *  @return     The physical address of the allocated page
 */
unsigned int pmm_alloc() {
    for (unsigned int i = 1; i < MAX_PAGES; i++) {
        if (bitmap_test(i) == 0) {
            bitmap_set(i);
            return i * PAGE_SIZE;
        }
    }
    return 0;  // fail
}

/** pmm_free:
 *  Marks a page as free in the bitmap
 *
 *  @param address  The physical address of the page to free
 */
void pmm_free(unsigned int address) {
    unsigned int index = address / PAGE_SIZE;
    if (index == 0 || index >= MAX_PAGES) return;
    bitmap_clear(index);
}
