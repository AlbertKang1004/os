#include "gdt.h"

struct gdt_entry gdt[6] __attribute__((aligned(8)));
unsigned int gdt_size = sizeof(gdt);

/**
 * gdt_set_entry:
 *   Sets a GDT entry at the given index with the specified parameters.
 *
 * @param index         Index of the GDT entry to set
 * @param base          Base address of the segment
 * @param limit         Limit of the segment
 * @param access        Access byte (type, DPL, present bit)
 * @param granularity   Granularity byte (flags + limit 19:16)
 */
void gdt_set_entry(int index, unsigned int base, unsigned int limit, unsigned char access, unsigned char granularity) {
    gdt[index].base_low    = base & 0xFFFF;
    gdt[index].base_middle = (base >> 16) & 0xFF;
    gdt[index].base_high   = (base >> 24) & 0xFF;
    gdt[index].limit_low   = limit & 0xFFFF;
    gdt[index].granularity = (limit >> 16) & 0x0F;
    gdt[index].granularity |= granularity & 0xF0;
    gdt[index].access      = access;
}

/**
 * gdt_flush:
 *   Loads the GDT into the CPU using the LGDT instruction.
 *   Updates all segment registers to use the new GDT entries.
 *   Performs a far jump to reload the code segment (CS) register.
 *
 * @param gdt_desc      Pointer to the GDT descriptor containing size and address
 */
void gdt_flush(struct gdt_descriptor *gdt_desc) {
    __asm__ volatile (
        "lgdt (%0)\n"
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%ss\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "jmp $0x08, $1f\n"
        "1:\n"
        : : "r"(gdt_desc) : "eax"
    );
}