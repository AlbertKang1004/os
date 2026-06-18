#include "tss.h"
#include "gdt.h"
#include "utils.h"

/* The single global TSS. kmain points the GDT TSS descriptor (index 5) at
 * this address, so it must be visible there -- see tss.h's extern. */
struct tss_entry tss;

/**
 * tss_init:
 *   See tss.h.
 *
 * Build guide:
 *   - zero the whole tss struct first (kmemset(&tss, 0, sizeof(tss)))
 *   - tss.ss0  = KERNEL_DATA_SELECTOR
 *   - tss.esp0 = kernel_stack_top
 *   - tss.iomap_base = sizeof(tss)   (no I/O permission bitmap)
 *   - load the task register with TSS_SELECTOR:
 *       __asm__ volatile("ltr %0" : : "r"((unsigned short)TSS_SELECTOR));
 *     (the GDT TSS descriptor must already be installed + lgdt'd before this)
 */
void tss_init(unsigned int kernel_stack_top) {
    // TODO: your implementation
}

/**
 * tss_set_kernel_stack:
 *   See tss.h.
 *
 * Build guide:
 *   - tss.esp0 = kernel_stack_top;   (just update the field, no ltr needed)
 */
void tss_set_kernel_stack(unsigned int kernel_stack_top) {
    // TODO: your implementation
}
