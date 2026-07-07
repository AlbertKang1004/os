#include "scheduler.h"
#include "process.h"

static struct process *current = 0;
static struct process *tail = 0;

static inline void write_cr3(unsigned int page_dir_phys_addr) {
    asm volatile("mov %0, %%cr3" :: "r"(page_dir_phys_addr) : "memory");
}

/**
 * scheduler_add:
 *   Adds a process to the scheduler's ready queue (a circular linked list),
 *   making it eligible to be picked on the next context switch. Links the
 *   process into the ring via its `next` field and updates the queue's tail.
 *
 * @param p  The process to enqueue (must be a valid, fully created PCB).
 */
void scheduler_add(struct process * p) {
    if (current == 0) { // no process in queue
        current = p;
        tail = p; // last initialized process
        p->next = current;
    } else {
        p->next = tail->next;
        tail->next = p;
        tail = p;
    }
}

/**
 * schedule:
 *   Performs a round-robin context switch. Saves the outgoing process's
 *   register/stack state (from `cpu` and `stack`) into its PCB, advances
 *   `current` to the next process in the ready ring, then loads that
 *   process's saved state back into `cpu`/`stack`, switches the address
 *   space (cr3) and the kernel stack (tss.esp0). Called from the timer
 *   IRQ0 handler so the switch takes effect on `iret`.
 *
 * @param cpu    Saved general-purpose registers of the interrupted process.
 * @param stack  Saved iret frame (eip, cs, eflags, user esp/ss) to resume into.
 */
void schedule(struct cpu_state *cpu, struct stack_state *stack) {

    // case 1: USER MODE -> USER MODE
    current->cpu = *cpu;
    current->stack = *stack;

    current = current->next;
    write_cr3(current->page_directory); // switch page directory, flushing TLB
    
    *cpu = current->cpu;        // load saved cpu state
    *stack = current->stack;    // load saved stack state
    // tss.esp0 remains the same since all the process use the same kernel stack

    // TO BE IMPLEMENTED
    
    // case 2: USER MODE -> KERNEL MODE

    // case 3: KERNEL MODE -> USER MODE

    // case 4: KERNEL MODE -> KERNEL MODE
}

