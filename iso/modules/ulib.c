#include "../../include/syscall_nums.h"

int syscall3(int num, int a, int b, int c) {
    int ret;
    __asm__ volatile ("int $0x80" : 
        "=a"(ret) : "a"(num), "b"(a), "c"(b), "d"(c) : "memory");
    return ret;
}
int write(int fd, const void *buf, unsigned n) {
    return syscall3(SYS_WRITE, fd, (int)buf, n);
}

__attribute__((noreturn)) 
void exit(int status) {
    syscall3(SYS_EXIT, status, 0, 0);
    for (;;); // cannot go through
}