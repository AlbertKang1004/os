#ifndef ULIB_H
#define ULIB_H

int syscall3(int num, int a, int b, int c);
int write(int fd, const void *buf, unsigned n);
int sleep(unsigned int seconds);
void exit(int status);

#endif