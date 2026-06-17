#include "vmm.h"
#include "pmm.h"
#include "utils.h"

/**
 * vmm_map_page:
 *   Maps a physical frame to a virtual address in the current page directory.
 *
 * @param phys      Physical address of the frame to map (must be 4KB aligned)
 * @param virt      Virtual address to map to (must be 4KB aligned)
 * @param flags     Page table entry flags (e.g. PAGE_PRESENT | PAGE_RW | PAGE_USER)
 *
 * @return          0 on success, -1 on failure (e.g. pmm_alloc failed)
 */
int vmm_map_page(unsigned int phys, unsigned int virt, unsigned int flags) {
    // Extract index from virtual address
    unsigned int pd_index = virt >> 22;
    unsigned int pt_index = (virt >> 12) & 0x3FF;

    if (!(PD_VIRT[pd_index] & PAGE_PRESENT)) { // page table does not exist
        unsigned int new_pt_phys = pmm_alloc();
        if (new_pt_phys == 0) return -1; // out of memory

        // install new table in directory (must be present before we touch its window)
        PD_VIRT[pd_index] = new_pt_phys | PAGE_PRESENT | PAGE_RW;
        tlb_flush((unsigned int) PT_VIRT(pd_index)); // PDE changed -> refresh window
        kmemset(PT_VIRT(pd_index), 0, 4096); // zero the new table
    }

    // write the mapping virt -> phys
    PT_VIRT(pd_index)[pt_index] = phys | flags | PAGE_PRESENT;
    tlb_flush(virt); // refresh the mapped address
    return 0;
}

/**
 * vmm_unmap_page:
 *   Unmaps a virtual address from the current page directory.
 *   Flushes the TLB entry for the given virtual address.
 *
 * @param virt      Virtual address to unmap (must be 4KB aligned)
 */
void vmm_unmap_page(unsigned int virt) {
    // Extract index from virtual address
    unsigned int pd_index = virt >> 22;
    unsigned int pt_index = (virt >> 12) & 0x3FF;

    // if page directory entry is empty -> return
    if (!(PD_VIRT[pd_index] & PAGE_PRESENT)) return;

    // clear the PTE for virt via the recursive window (PT_VIRT)
    PT_VIRT(pd_index)[pt_index] = 0;

    // if page directory contains no page table entry, it should freed that address
    if (pt_is_empty(pd_index)) {
        PD_VIRT[pd_index] = 0;
    }
    
    // refresh the unmapped address so the CPU drops the old translation
    tlb_flush(virt);
}

/**
 * vmm_get_phys:
 *   Resolves a virtual address to its mapped physical address.
 *
 * @param virt      Virtual address to resolve
 *
 * @return          Physical address on success, 0 if not mapped
 */
unsigned int vmm_get_phys(unsigned int virt) {
    // Extract index from virtual address
    unsigned int pd_index = virt >> 22;
    unsigned int pt_index = (virt >> 12) & 0x3FF;

    // if the page directory entry is empty -> not mapped, return 0
    if (!(PD_VIRT[pd_index] & PAGE_PRESENT)) return 0;

    // read the PTE via the recursive window; if not present -> not mapped
    if (!(PT_VIRT(pd_index)[pt_index] & PAGE_PRESENT)) return 0;

    // mask off the low 12 flag bits and return the physical frame address
    unsigned int phys_addr = PT_VIRT(pd_index)[pt_index] & ~0xFFF;
    return phys_addr;
}

static int pt_is_empty(unsigned int pd_index) {
    for (int i = 0; i < 1024; i++)
        if (PT_VIRT(pd_index)[i] & PAGE_PRESENT) return 0;  // found one -> not empty
    return 1;  // got through all 1024 -> empty
}

