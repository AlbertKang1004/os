#include "interrupt.h"
#include "debug.h"

static interrupt_handler_t handler_table[256]; // init as 0s


void register_interrupt_handler(unsigned int num, interrupt_handler_t fn) {
    if (num > 255) return;
    handler_table[num] = fn; // save the function in the interrupt handler table
}

void interrupt_handler(struct cpu_state *cpu, struct stack_state *stack, unsigned int interrupt) {
    
    if (handler_table[interrupt] != 0) {
        handler_table[interrupt](cpu, stack, interrupt);
    } else {
        LOG_HEX("Interrupt", interrupt);
        LOG_HEX("Code", stack->error_code);
    }
}



