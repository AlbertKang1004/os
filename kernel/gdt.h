#ifndef GDT_H
#define GDT_H

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