#ifndef PROCESS_H
#define PROCESS_H

#include "interrupt.h"

#define FD_MAX 256

enum process_state {
    PROCESS_READY,
    PROCESS_SLEEPING,
    PROCESS_BLOCKED
} __attribute__((packed));

enum fd_type {
    FD_NONE,
    FD_TAR_FILE,
    FD_SERIAL,
    FD_KEYBOARD
} __attribute__((packed));

struct fd {
    enum fd_type type;              // type of file descriptor
    // unsigned int in_use;         // 1 if in use, 0 if not --> not needed as the table is array of pointers
    unsigned int data;              // starting address, in virtual
    unsigned int size;              // size of the file
    unsigned int offset;            // current offset
    unsigned int flags;             // flags
} __attribute__((packed));

struct process {
    unsigned int page_directory;  // page directory for the process
    unsigned int code_addr;       // code virtual address
    unsigned int stack_addr;      // stack virtual address
    unsigned int wake_tick;       // only matters when process is sleeping
    enum process_state state;     // current state of the process
    struct cpu_state cpu;         // saved cpu state
    struct stack_state stack;     // saved stack state
    struct fd * fd_table[FD_MAX];     // file descriptor array
    struct process * next;        // next process
} __attribute__((packed));

struct process *process_create(unsigned int binary_start, unsigned int binary_size);

#endif