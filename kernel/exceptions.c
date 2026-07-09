#include "cpu.h"
#include "debug.h"
#include "interrupt.h"

static void page_fault_handler(struct cpu_state *cpu, struct stack_state *stack, unsigned int interrupt) {
    (void) cpu;
    (void) interrupt;
    unsigned int cr2 = read_cr2();
    LOG_HEX("cr2", cr2);           // faulting address (valid for page fault)
    LOG_HEX("fault_eip", stack->eip); // instruction that caused the fault

    for (;;) {  // permanently freeze the kernel
        cli();
        hlt();
    }
}

void page_fault_init(void) {
    register_interrupt_handler(0xE, page_fault_handler);
}