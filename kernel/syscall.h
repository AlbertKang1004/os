#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_TABLE_SIZE 256

#include "interrupt.h"

typedef int (*syscall_handler_t) (struct cpu_state *, struct stack_state *);

int sys_read(struct cpu_state * cpu, struct stack_state * stack);
int sys_write(struct cpu_state * cpu, struct stack_state * stack);
int sys_open(struct cpu_state * cpu, struct stack_state * stack);
int sys_close(struct cpu_state * cpu, struct stack_state * stack);
int sys_exit(struct cpu_state * cpu, struct stack_state * stack);
int sys_nanosleep(struct cpu_state * cpu, struct stack_state * stack);
void syscall_init(void);

#endif