#ifndef ULIB_H
#define ULIB_H

int syscall3(int num, int a, int b, int c);

int read(int fd, void *buf, unsigned int count);
int write(int fd, const void *buf, unsigned int n);
int open(const char *filename, int flags, int mode);
int close(int fd);
int sleep(unsigned int seconds);
void exit(int status);

#endif