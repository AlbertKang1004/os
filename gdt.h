#ifndef INCLUDE_GDT_H
#define INCLUDE_GDT_H

struct gdt_descriptor {
    unsigned short size;
    unsigned int address;
} __attribute__((packed));

/** gdt_flush:
 *  Loads the GDT and updates the segment registers
 * 
 *  @param gdt_addr     The address of the GDT Descriptor
 */
void gdt_flush(struct gdt_descriptor *gdt_addr);

#endif