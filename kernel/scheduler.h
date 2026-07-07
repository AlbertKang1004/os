#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"
#include "interrupt.h"

void scheduler_add(struct process * p);
void schedule(struct cpu_state *cpu, struct stack_state *stack);

#endif