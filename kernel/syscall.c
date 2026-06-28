#include "syscall.h"
#include "interrupt.h"
#include "../lib/io.h"
#include "../drivers/serial.h"

syscall_handler_t syscall_table[SYSCALL_TABLE_SIZE] = {
    [0] = sys_read,
    [1] = sys_write,
    [2] = sys_open,
    [3] = sys_close,
    [4] = sys_exit
};

int sys_read(unsigned int ebx, unsigned int ecx, unsigned int edx) {
    unsigned int fd = ebx;
    char *buf = (char *) ecx;
    unsigned int count = edx;
    return 0;
}

int sys_write(unsigned int ebx, unsigned int ecx, unsigned int edx) {
    unsigned int fd = ebx;
    const char *buf = (const char *) ecx;
    unsigned int count = edx;
    for (unsigned int i = 0; i < count; i++) {
        outb(SERIAL_COM1_BASE, buf[i]);
    }
    return 0;
}

int sys_open(unsigned int ebx, unsigned int ecx, unsigned int edx) {
    const char *filename = (const char *) ebx;
    int flags = ecx;
    int mode = edx;
    return 0;
}

int sys_close(unsigned int ebx, unsigned int ecx, unsigned int edx) {
    unsigned int fd = ebx;
    (void) ecx;
    (void) edx;
    return 0;
}

int sys_exit(unsigned int ebx, unsigned int ecx, unsigned int edx) {
    int error_code = ebx;
    (void) ecx;
    (void) edx;
    return 0;
}

void syscall_dispatch(struct cpu_state * cpu) {
    unsigned int syscall_num = cpu->eax;
    if (syscall_num >= SYSCALL_TABLE_SIZE || syscall_table[syscall_num] == 0) { 
        cpu->eax = -1;
    } else {
        cpu->eax = syscall_table[syscall_num](cpu->ebx, cpu->ecx, cpu->edx);
    }
}