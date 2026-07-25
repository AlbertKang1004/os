#include "syscall.h"
#include "interrupt.h"
#include "../drivers/pit.h"
#include "../lib/io.h"
#include "../drivers/serial.h"
#include "../include/syscall_nums.h"
#include "scheduler.h"
#include "debug.h"
#include "initrd.h"
#include "process.h"
#include "kmalloc.h"

extern unsigned int multiboot_info_ptr;

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
    char *buf = (char *)(unsigned long) cpu->ecx;
    unsigned int count = cpu->edx;

    struct process *proc = scheduler_current();
    if (fd >= FD_MAX || proc->fd_table[fd] == 0) return -1;
    switch (proc->fd_table[fd]->type) {
        case FD_SERIAL: return -1;
        case FD_KEYBOARD: 
            return 0; // TODO
        case FD_TAR_FILE: {
            struct fd * current_fd = proc->fd_table[fd];
            unsigned char * pt = (unsigned char *)(unsigned long) current_fd->data + current_fd->offset;
            if (count > current_fd->size - current_fd->offset)
                count = current_fd->size - current_fd->offset;
            kmemcpy(buf, pt, count);
            current_fd->offset += count;
            return count;
        }
        case FD_NONE: return -1;
        default: return -1;
    }
}

int sys_write(struct cpu_state * cpu, struct stack_state * stack) {
    (void) stack;
    unsigned int fd = cpu->ebx;
    const char *buf = (const char *)(unsigned long) cpu->ecx;
    unsigned int count = cpu->edx;


    struct process *proc = scheduler_current();
    if (fd >= FD_MAX || proc->fd_table[fd] == 0) return -1;
    switch (proc->fd_table[fd]->type) {
        case FD_SERIAL: 
            for (unsigned int i = 0; i < count; i++) {
                outb(SERIAL_COM1_BASE, buf[i]);
            }
            return count;
        case FD_KEYBOARD: 
        case FD_TAR_FILE: 
        case FD_NONE: 
        default: return -1;
    }
}

int sys_open(struct cpu_state * cpu, struct stack_state * stack) {
    (void) stack;
    const char *filename = (const char *)(unsigned long) cpu->ebx;
    int flags = cpu->ecx;
    int mode = cpu->edx;
    char *out;
    int file_size = tar_lookup(filename, &out);
    if (file_size == -1) { return -1; }
    struct process *proc = scheduler_current();
    int i;
    for (i = 0; i < FD_MAX && proc->fd_table[i] != 0; i++);
    if (i >= FD_MAX) return -1;

    if ((proc->fd_table[i] = kmalloc(sizeof(*proc->fd_table[i]))) == 0) {
        return -1; // failed to allocate space
    }

    struct fd * current_fd = proc->fd_table[i];
    current_fd->type = FD_TAR_FILE;
    current_fd->data = (unsigned int)(unsigned long) out;
    current_fd->size = file_size;
    current_fd->offset = 0;
    current_fd->flags = 0;

    return i;
}

int sys_close(struct cpu_state * cpu, struct stack_state * stack) {
    (void) stack;
    unsigned int fd = cpu->ebx;
    struct process *proc = scheduler_current();

    if (fd >= FD_MAX || proc->fd_table[fd] == 0) return -1;
    kfree(proc->fd_table[fd]);
    proc->fd_table[fd] = 0;
    
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