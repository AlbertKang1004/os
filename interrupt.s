global load_idt

%macro no_error_code_interrupt_handler %1
global interrupt_handler_%1
interrupt_handler_%1:
    push    dword 0                     ; push 0 as error code
    push    dword %1                    ; push the interrupt number
    jmp     common_interrupt_handler    ; jump to the common handler
%endmacro

%macro error_code_interrupt_handler %1
global interrupt_handler_%1
interrupt_handler_%1:
    push    dword %1                    ; push the interrupt number
    jmp     common_interrupt_handler    ; jump to the common handler
%endmacro

; load_idt - Loads the interrupt descriptor table (IDT).
; stack: [esp + 4] the address of the first entry in the IDT
;        [esp    ] the return address
load_idt:
    mov     eax, [esp+4]    ; load the address of the IDT into register eax
    lidt    eax             ; load the IDT
    ret                     ; return to the calling function

common_interrupt_handler:               ; the common parts of the generic interrupt handler
    pushad                              ; save the registers
    mov eax, [esp + 32]
    push [esp + 32]                     ; argument 3 - interrupt

    mov eax, [esp + 40]
    push [esp + 40]                     ; argument 2 - stack_state

    mov eax, [esp + 8]
    push esp                            ; argument 1 - cpu_state
    call    interrupt_handler           ; call the C function

    add esp, 12                         ; remove the arguments
    popad                               ; restore the registers
    add     esp, 8                      ; restore the esp
    iret                                ; return to the code that got interrupted

%assign i 0
%rep 8
    no_error_code_interrupt_handler i
    %assign i i+1
%endrep

error_code_interrupt_handler    8
no_error_code_interrupt_handler 9

%assign i 10
%rep 5
    error_code_interrupt_handler i
    %assign i i+1
%endrep

no_error_code_interrupt_handler 15
no_error_code_interrupt_handler 16
error_code_interrupt_handler    17

%assign i 18
%rep 238
    no_error_code_interrupt_handler i
    %assign i i+1
%endrep