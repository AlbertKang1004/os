#ifndef UTILS_H
#define UTILS_H

/* The Colors usable by text */
#define COLOR_BLACK         0
#define COLOR_BLUE          1
#define COLOR_GREEN         2
#define COLOR_CYAN          3
#define COLOR_RED           4
#define COLOR_MAGENTA       5
#define COLOR_BROWN         6
#define COLOR_LIGHT_GREY    7
#define COLOR_DARK_GREY     8
#define COLOR_LIGHT_BLUE    9
#define COLOR_LIGHT_GREEN   10
#define COLOR_LIGHT_CYAN    11
#define COLOR_LIGHT_RED     12
#define COLOR_LIGHT_MAGENTA 13
#define COLOR_LIGHT_BROWN   14
#define COLOR_WHITE         15

char *print_hex(unsigned int n);
void kmemset(void *ptr, int value, unsigned int size);
void *kmemcpy(void *dest, const void *src, unsigned int size);
int kstrcmp(const char *s1, const char *s2);
int kstrncmp(const char *s1, const char *s2, unsigned int n);
int kstrlen(const char *s);

#endif