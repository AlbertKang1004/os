#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_TABLE_SIZE 256

#include "interrupt.h"

typedef int (*syscall_handler_t) (unsigned int, unsigned int, unsigned int);

int sys_read(unsigned int ebx, unsigned int ecx, unsigned int edx);
int sys_write(unsigned int ebx, unsigned int ecx, unsigned int edx);
int sys_open(unsigned int ebx, unsigned int ecx, unsigned int edx);
int sys_close(unsigned int ebx, unsigned int ecx, unsigned int edx);
int sys_exit(unsigned int ebx, unsigned int ecx, unsigned int edx);
void syscall_init(void);

#endif