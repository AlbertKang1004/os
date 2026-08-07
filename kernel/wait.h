#ifndef WAIT_H
#define WAIT_H

struct process;
struct cpu_state;
struct stack_state;
struct wait_queue {
    struct process *head;
};

void wait_queue_block(struct wait_queue * wq, struct cpu_state * cpu, struct stack_state * stack);
void wait_queue_wake(struct wait_queue * wq);

#endif