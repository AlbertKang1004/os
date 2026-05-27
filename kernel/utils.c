#include "utils.h"

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