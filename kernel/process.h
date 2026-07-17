#ifndef PROCESS_H
#define PROCESS_H

#include "interrupt.h"

enum process_state {
    PROCESS_READY,
    PROCESS_SLEEPING
} __attribute__((packed));

struct process {
    unsigned int page_directory;  // page directory for the process
    unsigned int code_addr;       // code virtual address
    unsigned int stack_addr;      // stack virtual address
    unsigned int wake_tick;       // only matters when process is sleeping
    enum process_state state;     // current state of the process
    struct cpu_state cpu;         // saved cpu state
    struct stack_state stack;     // saved stack state
    struct process * next;        // next process
} __attribute__((packed));

struct process *process_create(unsigned int binary_start, unsigned int binary_size);

#endif