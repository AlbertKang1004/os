#include "syscall.h"
#include "interrupt.h"
#include "../drivers/pit.h"
#include "../lib/io.h"
#include "../drivers/serial.h"
#include "../include/syscall_nums.h"
#include "scheduler.h"
#include "debug.h"

syscall_handler_t syscall_table[SYSCALL_TABLE_SIZE] = {
    [0] = sys_read,
    [1] = sys_write,
    [2] = sys_open,
    [3] = sys_close,
    [4] = sys_exit,
    [5] = sys_nanosleep
};

int sys_read(struct cpu_state * cpu, struct stack_state * stack) {
    (void) stack;
    unsigned int fd = cpu->ebx;
    char *buf = (char *) cpu->ecx;
    unsigned int count = cpu->edx;
    return 0;
}

int sys_write(struct cpu_state * cpu, struct stack_state * stack) {
    (void) stack;
    unsigned int fd = cpu->ebx;
    const char *buf = (const char *) cpu->ecx;
    unsigned int count = cpu->edx;
    for (unsigned int i = 0; i < count; i++) {
        outb(SERIAL_COM1_BASE, buf[i]);
    }
    return 0;
}

int sys_open(struct cpu_state * cpu, struct stack_state * stack) {
    (void) stack;
    const char *filename = (const char *) cpu->ebx;
    int flags = cpu->ecx;
    int mode = cpu->edx;
    return 0;
}

int sys_close(struct cpu_state * cpu, struct stack_state * stack) {
    (void) stack;
    unsigned int fd = cpu->ebx;
    return 0;
}

int sys_exit(struct cpu_state * cpu, struct stack_state * stack) {
    int status = cpu->ebx;
    LOG_HEX("process exited, status", status);
    scheduler_exit_current(cpu, stack);
    return 0; // not used anyways
}

int sys_nanosleep(struct cpu_state * cpu, struct stack_state * stack) {
    unsigned int tick = cpu->ebx * 1000;
    cpu->eax = 0;
    scheduler_sleep_current(cpu, stack, tick);
    return 0;
}

static void syscall_dispatch(struct cpu_state * cpu, struct stack_state * stack, unsigned int interrupt) {
    (void) interrupt;

    unsigned int syscall_num = cpu->eax;
    if (syscall_num >= SYSCALL_TABLE_SIZE || syscall_table[syscall_num] == 0) { 
        cpu->eax = -1;
    } else if (syscall_num == SYS_EXIT || syscall_num == SYS_SLEEP) { 
        syscall_table[syscall_num](cpu, stack);
    } else {
        cpu->eax = syscall_table[syscall_num](cpu, stack);
    }
}

void syscall_init() {
    register_interrupt_handler(0x80, syscall_dispatch);
}