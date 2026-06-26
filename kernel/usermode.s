global enter_user_mode

section .text

enter_user_mode:
    ; extract values from stack
    mov eax, [esp + 8] ; user mode stack pointer
    mov ebx, [esp + 4] ; instruction pointer

    mov ecx, 0x23
    mov ds, ecx
    mov es, ecx
    mov fs, ecx
    mov gs, ecx

    push 0x23   ; ss
    push eax    ; esp
    push 0x202  ; eflags
    push 0x1B   ; cs
    push ebx    ; eip
    iret

; extern void enter_user_mode(unsigned int entry, unsigned int user_stack);