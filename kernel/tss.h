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

void tss_init(unsigned int kernel_stack_top);
void tss_set_kernel_stack(unsigned int kernel_stack_top);

extern struct tss_entry tss;

#endif
