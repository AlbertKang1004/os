int main(void) {
    volatile unsigned int x = 0xCAFEBABE;
    (void) x;
    while (1) {}
    return 0;
}