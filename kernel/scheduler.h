#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"
#include "interrupt.h"

void scheduler_add(struct process * p);
void scheduler_set_idle(struct process *p);
void schedule(struct cpu_state *cpu, struct stack_state *stack);
void scheduler_exit_current(struct cpu_state *cpu, struct stack_state *stack);
void scheduler_sleep_current(struct cpu_state *cpu, struct stack_state *stack, unsigned int tick);
void scheduler_wake_blocked(void);
struct process *scheduler_current(void);

#endif