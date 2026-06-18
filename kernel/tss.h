#ifndef TSS_H
#define TSS_H

/*
 * Task State Segment (32-bit).
 * The CPU reads ss0/esp0 from here when a ring 3 -> ring 0 transition happens
 * (interrupt or syscall from user mode): it switches to that kernel stack
 * automatically. Layout is fixed by the hardware -- do not reorder.
 */
struct tss_entry {
    unsigned int prev_tss;   // unused (hardware task switching only)
    unsigned int esp0;       // kernel stack pointer loaded on ring change
    unsigned int ss0;        // kernel stack segment loaded on ring change
    unsigned int esp1;
    unsigned int ss1;
    unsigned int esp2;
    unsigned int ss2;
    unsigned int cr3;
    unsigned int eip;
    unsigned int eflags;
    unsigned int eax, ecx, edx, ebx;
    unsigned int esp, ebp, esi, edi;
    unsigned int es, cs, ss, ds, fs, gs;
    unsigned int ldt;
    unsigned short trap;
    unsigned short iomap_base;
} __attribute__((packed));

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
void tss_init(unsigned int kernel_stack_top);

/**
 * tss_set_kernel_stack:
 *   Updates esp0 (the kernel stack the CPU switches to on the next ring
 *   3 -> ring 0 transition). Call on each context switch with the incoming
 *   process's kernel stack.
 *
 * @param kernel_stack_top   New esp0 value
 */
void tss_set_kernel_stack(unsigned int kernel_stack_top);

/* The global TSS instance; kmain needs its address to build the GDT
 * TSS descriptor (index 5). */
extern struct tss_entry tss;

#endif
