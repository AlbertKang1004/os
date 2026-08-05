#include "ulib.h"

int main(void) {
    char buf[100];
    int fd = open("hello.txt", 0, 0);
    int n = read(fd, buf, sizeof(buf));
    write(1, buf, n);
    while (1) {
        n = read(0, buf, 100);
        write(1, buf, n);
    }
    close(fd);
    exit(0);
}