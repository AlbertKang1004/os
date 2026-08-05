#include "ulib.h"

int main(void) {
    while (1) {
        write(1, "tick\n", 5);
        sleep(1);
    }
}