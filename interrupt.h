#ifndef INCLUDE_INTERRUPT_H
#define INCLUDE_INTERRUPT_H

struct cpu_state {
    unsigned int edi;
    unsigned int esi;
    unsigned int ebp;
    unsigned int esp;
    unsigned int ebx;
    unsigned int edx;
    unsigned int ecx;
    unsigned int eax;
} __attribute__((packed));

struct stack_state {
    unsigned int error_code;
    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;
} __attribute__((packed));

struct idt_entry {
    unsigned short offset_low;
    unsigned short segment;
    unsigned char reserved;
    unsigned char type_attr;
    unsigned short offset_high;
} __attribute__((packed));

struct idt_descriptor {
    unsigned short size;
    unsigned int address;
} __attribute__((packed));

/** interrupt_handler:
 *  Handles the interrupt by delegating to the appropriate handler
 * 
 *  @param cpu          The CPU register state at the time of the interrupt
 *  @param stack        The stack state pushed by the CPU when the interrupt occurred
 *  @param interrupt    The interrupt number
 */
void interrupt_handler(struct cpu_state *cpu, 
    struct stack_state *stack, unsigned int interrupt);

/** load_idt:
 * Loads the Interrupt Descriptor Table.
 * 
 *  @param idt_addr     The address of IDT
 */
void load_idt(struct idt_descriptor *idt_addr);

#endif