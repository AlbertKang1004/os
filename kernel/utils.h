#ifndef UTILS_H
#define UTILS_H

char *print_hex(unsigned int n);
void kmemset(void *ptr, int value, unsigned int size);
void *kmemcpy(void *dest, const void *src, unsigned int size);
int kstrcmp(const char *s1, const char *s2);
int kstrncmp(const char *s1, const char *s2, unsigned int n);
int kstrlen(const char *s);

#endif