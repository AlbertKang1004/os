#include "ulib.h"

int main(void) {
    char* text = "hello!\n";
    write(0, text, 7);
    // *(volatile int *)0xDEADBEEF = 1; testing error
    while (1){}
}