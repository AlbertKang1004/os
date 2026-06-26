BITS 32

extern main
section .text
    ; push argv, argc
    call main
    ; main returned, and eax holds return value
    jmp $