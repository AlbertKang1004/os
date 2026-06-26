#include "tss.h"
#include "gdt.h"
#include "utils.h"

/* The single global TSS. kmain points the GDT TSS descriptor (index 5) at
 * this address, so it must be visible there -- see tss.h's extern. */
struct tss_entry tss;

/**
 * tss_init:
 *   Initializes the global TSS: sets ss0 to the kernel data selector and
 *   esp0 to the given kernel stack top, then loads the task register (ltr)
 *   with TSS_SELECTOR. Call once after the GDT (incl. the TSS descriptor)
 *   is loaded.
 *
 * @param kernel_stack_top   Initial esp0 (top of the kernel stack to use
 *                           when an interrupt arrives from user mode)
 */
void tss_init(unsigned int kernel_stack_top) {
    kmemset(&tss, 0, sizeof(tss));
    tss.ss0 = KERNEL_DATA_SELECTOR;
    tss.esp0 = kernel_stack_top;
    tss.iomap_base = sizeof(tss);
    __asm__ volatile("ltr %0" : : "r"((unsigned short) TSS_SELECTOR));
}

/**
 * tss_set_kernel_stack:
 *   Updates esp0 (the kernel stack the CPU switches to on the next ring
 *   3 -> ring 0 transition). Call on each context switch with the incoming
 *   process's kernel stack.
 *
 * @param kernel_stack_top   New esp0 value
 */
void tss_set_kernel_stack(unsigned int kernel_stack_top) {
    tss.esp0 = kernel_stack_top;
}
