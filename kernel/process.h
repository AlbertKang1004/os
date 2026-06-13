#ifndef PROCESS_H
#define PROCESS_H

struct process {
    unsigned int page_directory;  // page directory for the process
    unsigned int code_addr;       // code virtual address
    unsigned int stack_addr;      // stack virtual address
};

struct process *process_create(unsigned int binary_start, unsigned int binary_size);

#endif