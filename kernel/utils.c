#include "utils.h"

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