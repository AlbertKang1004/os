#include "utils.h"
#include "gdt.h"

/** print_hex:
 *  Converts an unsigned integer to a hexadecimal string
 *
 *  @param n    The unsigned integer to convert
 *  @return     The hexadecimal string representation of n
 */
char *print_hex(unsigned int n) {
    static char hex[] = "0x00000000";
    int i;
    for (i = 9; i >= 2; i--) {
        int digit = n & 0xF;
        hex[i] = digit < 10 ? '0' + digit : 'A' + digit - 10;
        n >>= 4;
    }
    return hex;
}

void kmemset(void *ptr, int value, unsigned int size) {
    unsigned char *p = (unsigned char *)ptr;
    for (unsigned int i = 0; i < size; i++) {
        p[i] = (unsigned char)value;
    }
}

/**
 * kmemcpy:
 *   Copies size bytes from src to dest. The two regions must not overlap.
 *
 * @param dest      Destination buffer to copy into
 * @param src       Source buffer to copy from
 * @param size      Number of bytes to copy
 * @return          dest (the destination pointer)
 */
void *kmemcpy(void *dest, const void *src, unsigned int size) {
    unsigned char * d = (unsigned char *) dest;
    unsigned const char * s = (unsigned const char *) src;
    for (unsigned int i = 0 ; i < size ; i++) {
        d[i] = s[i];
    }
    return d;
}

/**
 * kstrcmp:
 *   Compares two null-terminated strings lexicographically.
 *
 * @param s1    First null-terminated string
 * @param s2    Second null-terminated string
 * @return      0 if the strings are equal;
 *              a negative value if s1 is less than s2;
 *              a positive value if s1 is greater than s2
 */
int kstrcmp(const char *s1, const char *s2) {
    while (*s1 != 0 && *s2 != 0) {
        if (*s1 != *s2) { // two strings are different
            return (int) *s1 - *s2;
        }   
        s1++;
        s2++;
    }
    if (*s1 == *s2) {
        return 0;
    }
    else {
        return (int) *s1 - *s2;
    }    
}

int kstrncmp(const char *s1, const char *s2, unsigned int n) {
    while (*s1 != 0 && *s2 != 0 && n > 0) {
        if (*s1 != *s2) { // two strings are different
            return (int) *s1 - *s2;
        }   
        s1++;
        s2++;
        n--;
    }
    if (n > 0) {
        return (int) *s1 - *s2;
    } else {
        return 0;
    }
}

int kstrlen(const char *s) {
    int count = 0;
    while (*s != 0) {
        s++, count++;
    }
    return count;
}