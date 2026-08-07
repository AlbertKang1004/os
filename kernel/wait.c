#include "wait.h"
#include "interrupt.h"
#include "scheduler.h"

/**
 * wait_queue_block:
 *   Puts the calling process to sleep on `wq` until someone wakes that
 *   queue. Marks it BLOCKED so pick_next skips it, links it into the
 *   queue so the waker can find it, rewinds the saved eip back over the
 *   `int 0x80` so the syscall re-runs on wake, then hands the CPU away.
 *
 *   Order matters: the rewind must happen BEFORE schedule(), which copies
 *   the trap frame into the PCB. Rewinding afterwards would write into
 *   the incoming process's frame instead, and this process would resume
 *   past a syscall it never completed.
 *
 *   The syscall restarts rather than resumes, because there is one shared
 *   kernel stack and nowhere to park this one's C frame. Callers must
 *   therefore block only before consuming anything -- a partial read has
 *   to return what it already has, or those bytes are lost when the call
 *   runs again.
 *
 *   Does not return in the usual sense: by the time it does, `cpu` and
 *   `stack` describe a DIFFERENT process, so the caller must not write
 *   through them afterwards. syscall_dispatch detects this by comparing
 *   scheduler_current() across the handler.
 *
 * @param wq     Queue to sleep on.
 * @param cpu    Live cpu_state on the kernel stack.
 * @param stack  Live iret frame; its eip is rewound before the switch.
 */
void wait_queue_block(struct wait_queue * wq, struct cpu_state * cpu, struct stack_state * stack) {
    struct process * p = scheduler_current();
    p->state = PROCESS_BLOCKED;
    
    p->wait_next = wq->head;
    wq->head = p;
    stack->eip -= 2;
    schedule(cpu, stack);
}

/**
 * wait_queue_wake:
 *   Marks every process on `wq` READY and empties the queue. Called from
 *   interrupt context by whoever made the awaited condition true, so it
 *   only flips state -- the actual switch happens on the next timer tick.
 *
 *   Wakes all waiters rather than picking one. Since a blocked syscall
 *   restarts from scratch and re-checks its condition, waking a process
 *   that cannot make progress is harmless: it simply blocks again. That
 *   removes any obligation to prove no wakeup was lost, which is the
 *   subtle failure mode of waking a single waiter. Worth revisiting only
 *   if a queue ever holds enough waiters for the redundant restarts to
 *   matter.
 *
 *   Each entry's link is read before it is cleared -- clearing first
 *   would drop the rest of the list.
 *
 * @param wq  Queue to drain. Safe to call when empty.
 */
void wait_queue_wake(struct wait_queue * wq) {
    struct process * cur = wq->head;
    while (cur != 0) {
        cur->state = PROCESS_READY;
        struct process * next = cur->wait_next;
        cur->wait_next = 0;
        cur = next;
    }
    wq->head = 0;
}