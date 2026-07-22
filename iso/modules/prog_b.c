#include "ulib.h"

int main(void) {
    char* text = "world!\n";
    write(0, text, 7);
    sleep(2);
    write(0, text, 7);
    exit(1);
}