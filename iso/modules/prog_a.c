#include "ulib.h"

int main(void) {
    char * buf[100];
    int fd = open("hello.txt", 0, 0);
    int n = read(fd, buf, sizeof(buf));
    write(1, buf, n);
    close(fd);
    return 0;
}