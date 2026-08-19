#ifndef ULIB_H
#define ULIB_H

#include "../../include/syscall_nums.h"

int syscall3(int num, int a, int b, int c);

int read(int fd, void *buf, unsigned int count);
int write(int fd, const void *buf, unsigned int n);
int open(const char *filename, int flags, int mode);
int close(int fd);
int sleep(unsigned int seconds);
__attribute__((noreturn)) void exit(int status);

int strcmp(const char *s1, const char *s2);
int strlen(const char *s);
int puts(char* s);

#endif