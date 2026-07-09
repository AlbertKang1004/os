#include "../lib/io.h"
#include "../kernel/debug.h"
#include "../kernel/interrupt.h"
#include "keyboard.h"
#include "pic.h"

unsigned char read_scan_code(void) {
    return inb(KBD_DATA_PORT);
}

static void keyboard_interrupt_handler(struct cpu_state * cpu, struct stack_state * stack, unsigned int interrupt) {
    (void) cpu;
    (void) stack;
    // Keyboard interrupt
    unsigned char scan_code = read_scan_code();
    LOG_HEX("Key", scan_code);
    pic_acknowledge(interrupt);
}

void keyboard_init() {
    register_interrupt_handler(0x21, keyboard_interrupt_handler);
}