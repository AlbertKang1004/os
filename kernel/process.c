#include "process.h"
#include "pmm.h"
#include "vmm.h"

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

    for (unsigned i = 768; i < 1023 ; i++) {
       new_pd_virt[i] = page_directory[i];
    }
    vmm_unmap_page(0xC0400000);
    tlb_flush(0xC0400000);
    
}

// 0000 