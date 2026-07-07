#include "ulib.h"

int main(void) {
    char* text = "world!\n";
    write(0, text, 7);
    while (1){}
}