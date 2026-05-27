#ifndef PMM_H
#define PMM_H

#define PAGE_SIZE 0x400000
#define MAX_PAGES 32 

struct mmap_entry {
    unsigned int size;      // size of this entry (excluding this field)
    unsigned int addr_low;  // lower 32 bits of base address
    unsigned int addr_high; // upper 32 bits of base address
    unsigned int len_low;   // lower 32 bits of length
    unsigned int len_high;  // upper 32 bits of length
    unsigned int type;      // 1 = available, other = reserved
} __attribute__((packed));

extern unsigned int page_bitmap;

void pmm_init(unsigned int map_addr, unsigned int map_length);
unsigned int pmm_alloc();
void pmm_free(unsigned int address);

#endif