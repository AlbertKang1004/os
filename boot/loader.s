; EXPORT TO KMAIN.C
global loader                       ; the entry symbol for ELF
global page_directory
global kernel_virtual_start
global kernel_virtual_end
global kernel_physical_start
global kernel_physical_end
global multiboot_mods_addr
global multiboot_mmap_addr
global multiboot_mmap_length

; IMPORT FROM LINK.LD
extern kmain                        ; the C kernel entry point
extern kernel_virtual_start
extern kernel_virtual_end
extern kernel_physical_start
extern kernel_physical_end

MAGIC_NUMBER        equ 0x1BADB002      ; multiboot magic number
ALIGN_MODULES       equ 0x00000001      ; tell GRUB to align modules on page boundaries
CHECKSUM            equ -(MAGIC_NUMBER + ALIGN_MODULES) ; checksum: magic + flags + checksum = 0
KERNEL_VIRTUAL_BASE equ 0xC0000000
KERNEL_STACK_SIZE   equ 4096          ; size of stack in bytes

section .bss
multiboot_mods_addr:    resd 1
multiboot_mmap_addr:    resd 1
multiboot_mmap_length:  resd 1

align 4096                          ; page directory must be 4KB aligned
page_directory:
    resb 4096                       ; reserve 4KB for page directory

align 4
kernel_stack:                       ; label points to beginning of stack memory
    resb KERNEL_STACK_SIZE          ; reserve stack for the kernel

section .text
    dd MAGIC_NUMBER                 ; write the magic number
    dd ALIGN_MODULES                ; write the align modules flag
    dd CHECKSUM                     ; write the checksum

loader:
    mov edi, ebx                    ; save multiboot struct address in edi

    mov eax, page_directory - KERNEL_VIRTUAL_BASE  ; physical address of page directory
    mov ecx, 1024

clear_loop:                         ; clear all page directory entries to 0
    dec ecx
    mov dword [eax + ecx * 4], 0x0
    jnz clear_loop

    mov dword [eax],             0x00000083  ; 0x00000000 -> 0x00000000
    mov dword [eax + (768 * 4)], 0x00000083  ; 0xC0000000 -> 0x00000000

    ; save multiboot info before paging
    mov esi, [edi + 24]                           ; mods_addr
    mov [multiboot_mods_addr - KERNEL_VIRTUAL_BASE], esi
    mov esi, [edi + 44]                           ; mmap_length
    mov [multiboot_mmap_length - KERNEL_VIRTUAL_BASE], esi
    mov esi, [edi + 48]                           ; mmap_addr
    mov [multiboot_mmap_addr - KERNEL_VIRTUAL_BASE], esi

    mov cr3, eax                    ; load page directory address into cr3

    mov ebx, cr4                    ; read current cr4
    or  ebx, 0x00000010             ; set PSE bit (enable 4MB pages)
    mov cr4, ebx                    ; update cr4

    mov ebx, cr0                    ; read current cr0
    or  ebx, 0x80000000             ; set PG bit (enable paging)
    mov cr0, ebx                    ; update cr0

    lea ebx, [higher_half]
    jmp ebx
    
higher_half:
    ; mov dword [page_directory - 0xC0000000], 0x0
    ; invlpg [0]
    ; this is dealed by kmain.c
    mov esp, kernel_stack + KERNEL_STACK_SIZE   ; set up the stack
    call kmain                      ; call the C kernel
.loop:
    jmp .loop                       ; loop forever if kmain returns