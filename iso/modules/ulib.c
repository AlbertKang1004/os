#include "../../include/syscall_nums.h"

int syscall3(int num, int a, int b, int c) {
    int ret;
    __asm__ volatile ("int $0x80" : 
        "=a"(ret) : "a"(num), "b"(a), "c"(b), "d"(c) : "memory");
    return ret;
}

int read(int fd, void *buf, unsigned int count) {
    return syscall3(SYS_READ, fd, (int)(unsigned long) buf, count);
}

int write(int fd, const void *buf, unsigned n) {
    return syscall3(SYS_WRITE, fd, (int)(unsigned long) buf, n);
}

int open(const char *filename, int flags, int mode) {
    return syscall3(SYS_OPEN, (int)(unsigned long) filename, flags, mode);
}

int close(int fd) {
    return syscall3(SYS_CLOSE, fd, 0, 0);
}

int sleep(unsigned int seconds) {
    return syscall3(SYS_SLEEP, seconds, 0, 0);
}

__attribute__((noreturn)) 
void exit(int status) {
    syscall3(SYS_EXIT, status, 0, 0);
    for (;;); // cannot go through
}