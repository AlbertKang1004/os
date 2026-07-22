#ifndef CPU_H
#define CPU_H

static inline void cli(void) { // Clear Interrupt Flag
    __asm__ volatile ("cli");
}

static inline void sti(void) { // Set Interrupt Flag
    __asm__ volatile ("sti");
}

static inline void hlt(void) { // Halt
    __asm__ volatile ("hlt");
}

static inline unsigned int read_cr2(void) {
    unsigned int val;
    __asm__ volatile ("mov %%cr2, %0" : "=r"(val));
    return val;
}

static inline unsigned int read_cr3(void) {
    unsigned int val;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(val));
    return val;
}

static inline void write_cr3(unsigned int page_dir_phys_addr) {
    asm volatile("mov %0, %%cr3" :: "r"(page_dir_phys_addr) : "memory");
}

/**
 * tlb_flush:
 *   Invalidates a single TLB entry for the given virtual address.
 *   Must be called after modifying a page table entry to ensure
 *   the CPU uses the updated mapping.
 *
 * @param virt  Virtual address whose TLB entry should be invalidated
 */
static inline void tlb_flush(unsigned int virt) {
    __asm__ volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

#endif