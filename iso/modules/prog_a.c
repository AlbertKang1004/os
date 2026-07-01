#include "ulib.h"

int main(void) {
    char* text = "hello!\n";
    write(0, text, 7);
    while (1){}
}