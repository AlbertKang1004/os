#ifndef GDT_H
#define GDT_H

/* Segment selectors: GDT byte offset OR'd with RPL (low 2 bits = privilege). */
#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10
#define USER_CODE_SELECTOR   (0x18 | 3)   /* index 3, RPL 3 -> 0x1B */
#define USER_DATA_SELECTOR   (0x20 | 3)   /* index 4, RPL 3 -> 0x23 */
#define TSS_SELECTOR         0x28         /* index 5, RPL 0          */

struct gdt_descriptor {
    unsigned short size;
    unsigned int address;
} __attribute__((packed));

struct gdt_entry {
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char  base_middle;
    unsigned char  access;
    unsigned char  granularity;
    unsigned char  base_high;
} __attribute__((packed));

void gdt_flush(struct gdt_descriptor *gdt_addr);
void gdt_set_entry(int index, unsigned int base, unsigned int limit, unsigned char access, unsigned char granularity);

#endif