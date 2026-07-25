#include "interrupt.h"
#include "kmalloc.h"
#include "process.h"
#include "pmm.h"
#include "vmm.h"
#include "utils.h"
#include "debug.h"
#include "cpu.h"

extern unsigned int page_directory[];

/**
 * process_create:
 *   Creates a new user mode process from a binary module.
 *   Allocates page frames for code and stack, copies the binary,
 *   sets up a new page directory with user mode mappings.
 *
 * @param binary_start      Physical start address of the binary (from GRUB module)
 * @param binary_size       Size of the binary in bytes
 * @return                  Pointer to the created process struct, or 0 on failure
 *
 * Steps:
 *   - allocate and initialize a new page directory
 *   - copy kernel page directory entries (768~1023) into new page directory
 *   - allocate page frames for code, copy binary into them, map at 0x00000000
 *   - allocate one page frame for stack, map at 0xBFFFF000
 *   - allocate process struct with kmalloc, fill in fields
 *   - return process struct
 */
struct process *process_create(unsigned int binary_start, unsigned int binary_size) {
    unsigned int new_pd_phys = pmm_alloc();
    if (new_pd_phys == 0) return 0; // if failed
    vmm_map_page(new_pd_phys, 0xC0400000, PAGE_RW | PAGE_PRESENT);
    tlb_flush(0xC0400000);

    // creating page table for accessing the higher half, and do temp mapping
    unsigned int * new_pd_virt = (unsigned int *)(unsigned long) 0xC0400000;

    // clean up the whole page directory before use.
    kmemset(new_pd_virt, 0, 4096);

    for (unsigned i = 768; i < 1023 ; i++) {
        if (page_directory[i] == 0) { // no page table yet
            unsigned int frame = pmm_alloc();
            page_directory[i] = frame | PAGE_PRESENT | PAGE_RW;
            tlb_flush((unsigned int) PT_VIRT(i));
            kmemset((void *)PT_VIRT(i), 0, 4096);
        }
       new_pd_virt[i] = page_directory[i];
    }
    new_pd_virt[1023] = new_pd_phys | PAGE_PRESENT | PAGE_RW;

    vmm_unmap_page(0xC0400000);
    tlb_flush(0xC0400000);

    // save cr3 address (Page Directory)
    unsigned int saved_cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(saved_cr3));

    // change cr3
    __asm__ volatile("mov %0, %%cr3" : : "r"(new_pd_phys) : "memory");
    unsigned int code_addr = pmm_alloc();
    unsigned int stack_addr = pmm_alloc();
    vmm_map_page(code_addr, 0x00000000, PAGE_USER | PAGE_RW | PAGE_PRESENT);
    vmm_map_page(stack_addr, 0xBFFFF000, PAGE_USER | PAGE_RW | PAGE_PRESENT);

    // copy binary 
    kmemcpy((void *) 0x00000000, (void *)(0xC0000000 + binary_start), binary_size);   
    // revert back cr3
    __asm__ volatile("mov %0, %%cr3" : : "r"(saved_cr3) : "memory");
    struct process * proc = kmalloc(sizeof(struct process));
    if (proc == 0) return 0;

    // initialization phase
    proc -> page_directory = new_pd_phys;
    proc -> code_addr = 0x00000000;
    proc -> stack_addr = 0xC0000000; // starts from top
    proc -> wake_tick = 0;
    proc -> state = PROCESS_READY;

    kmemset(proc->fd_table, 0, sizeof(struct fd *) * FD_MAX);
    for (int i = 0; i < 3; i++) {
        struct fd * fd_entry = kmalloc(sizeof(struct fd));
        if (fd_entry == 0) return 0;
        fd_entry -> type = (i == 0) ? FD_KEYBOARD : FD_SERIAL;
        fd_entry -> data = 0;
        fd_entry -> size = 0;
        fd_entry -> offset = 0;
        fd_entry -> flags = 0;
        proc -> fd_table[i] = fd_entry;
    }
    proc -> next = 0; // NULL

    // save default state of the process
    struct stack_state stack;
    stack.error_code = 0;
    stack.eip = proc->code_addr;
    stack.cs = 0x1B;
    stack.eflags = 0x202;
    stack.user_esp = proc->stack_addr;
    stack.user_ss = 0x23;
    proc->stack = stack;
    
    struct cpu_state cpu;
    cpu.edi = 0;
    cpu.esi = 0;
    cpu.ebp = 0;
    cpu.esp = 0;
    cpu.ebx = 0;
    cpu.edx = 0;
    cpu.ecx = 0;
    cpu.eax = 0;
    proc->cpu = cpu;

    return proc;
}