#include "cpu.h"
#include "debug.h"
#include "kmalloc.h"
#include "scheduler.h"
#include "process.h"
#include "../drivers/pit.h"

static struct process *current = 0;
static struct process *tail = 0;
static struct process *idle = 0;
static unsigned int process_count = 0;

/**
 * pick_next:
 *   Selects the next process to run, searching the ready ring from
 *   `start`. Returns the first process that is READY, waking any
 *   SLEEPING process whose wake_tick has been reached. If a full
 *   lap finds nothing runnable, returns the idle process.
 *
 *   Pure selection: does not modify `current`, cr3, or any trap
 *   frame -- the caller decides what to do with the result. The only
 *   side effect is marking a due sleeper READY.
 *
 * @param start  Ring member to start searching from (must be in the
 *               ready ring; the idle process is not a valid start).
 * @return       The chosen process; never 0 (falls back to idle).
 */
static struct process * pick_next(struct process *start) {

    for (unsigned int i = 0; i < process_count ; i++) {
        if (start->state == PROCESS_READY) {
            return start;
        } else if (start->state == PROCESS_SLEEPING && start->wake_tick <= get_current_tick()) {
            start->state = PROCESS_READY; 
            return start;
        } 
        start = start->next;
    } 

    return idle;
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
    process_count++;
}

/**
 * scheduler_set_idle:
 *   Registers the idle process, the fallback pick_next returns when
 *   no ring process is runnable. The idle process lives outside the
 *   ready ring (never added via scheduler_add), never sleeps, and
 *   never exits. Must be called before the first schedule().
 *
 * @param p  The idle process PCB (a fully created process).
 */
void scheduler_set_idle(struct process *p) {
    idle = p;
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

    if (current != idle) {
        current = current->next;
    } else {
        current = tail;
    }

    current = pick_next(current);

    write_cr3(current->page_directory); // switch page directory, flushing TLB
    
    *cpu = current->cpu;        // load saved cpu state
    *stack = current->stack;    // load saved stack state
    LOG("process switch");
    // tss.esp0 remains the same since all the process use the same kernel stack

    // TO BE IMPLEMENTED
    
    // case 2: USER MODE -> KERNEL MODE

    // case 3: KERNEL MODE -> USER MODE

    // case 4: KERNEL MODE -> KERNEL MODE
}

/**
 * scheduler_exit_current:
 *   Terminates the currently running process. Removes it from the
 *   ready ring (its state is not saved -- it will never resume),
 *   then switches to the next process by loading its saved state
 *   into `cpu`/`stack` so the pending iret resumes it. If this was
 *   the last process, halts the system.
 *
 *   Called from the syscall dispatcher (SYS_EXIT), so `cpu`/`stack`
 *   point into the kernel stack frame the iret will consume.
 *
 * @param cpu    Live cpu_state on the kernel stack; overwritten with
 *               the next process's saved registers.
 * @param stack  Live iret frame on the kernel stack; overwritten with
 *               the next process's saved eip/cs/eflags/esp/ss.
 */
void scheduler_exit_current(struct cpu_state *cpu, struct stack_state *stack) {
    struct process *temp = current;
    struct process *prev = current;
    // TODO: free PCB, page frames, page directory for the exiting process
    if (current->next == current) { // only 1 process in the list
        current = 0;
        tail = 0;
        LOG("No process, stop.");
        for (;;) { // stop all system, no process left
            cli();
            hlt();
        }
    }

    while (prev->next != temp) {
        prev = prev->next; // move to next entry
    }
    prev->next = current->next;
    if (temp == tail) {
        tail = prev->next;
    }
    current = pick_next(prev->next);    
    kfree(temp);
    process_count--;
    LOG("Moving to next process.");
    write_cr3(current->page_directory);
    *cpu = current->cpu;
    *stack = current->stack;
}

/**
 * scheduler_sleep_current:
 *   Puts the currently running process to sleep for the given number
 *   of ticks. Marks it SLEEPING with an absolute wake-up time, then
 *   calls schedule() to save its state and switch to the next runnable
 *   process. The process resumes (right after its sleep syscall) once
 *   schedule() finds it with wake_tick reached.
 *
 *   Called from the syscall dispatcher (SYS_NANOSLEEP), so `cpu`/`stack`
 *   point into the kernel stack frame the iret will consume.
 *
 * @param cpu    Live cpu_state on the kernel stack; saved into this
 *               process's PCB, then overwritten with the next process's.
 * @param stack  Live iret frame on the kernel stack; saved, then
 *               overwritten likewise.
 * @param tick   How long to sleep, in timer ticks (1 tick = 1ms at
 *               the current PIT frequency).
 */
void scheduler_sleep_current(struct cpu_state *cpu, struct stack_state *stack, unsigned int tick) {
    current->state = PROCESS_SLEEPING;
    current->wake_tick = get_current_tick() + tick;
    schedule(cpu, stack);
}

