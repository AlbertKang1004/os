#include "ulib.h"

int main(void) {
    char* text = "hello!\n";
    write(1, text, 7);
    sleep(1); // 1 seconds sleep
    write(1, text, 7);
    exit(0);
}