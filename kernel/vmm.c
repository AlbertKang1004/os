#include "vmm.h"
#include "pmm.h"
#include "utils.h"

#define PAGE_PRESENT    0x01
#define PAGE_RW         0x02
#define PAGE_USER       0x04
#define PAGE_KERNEL     0x00

extern unsigned int page_directory[];
extern unsigned int page_table[];

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

    if (!(page_directory[pd_index] & PAGE_PRESENT)) { // present bit false
        // page table does not exist
        unsigned int new_pt_phys = pmm_alloc();
        if (new_pt_phys == 0) return -1;
        
        // temporarily map and zero out the space
        page_table[1023] = new_pt_phys | PAGE_PRESENT | PAGE_RW;
        tlb_flush(0xC03FF000);

        // zero out the new page table 
        kmemset((void *)0xC03FF000, 0, 4096);

        // writing entry at 1023 while still mapped
        unsigned int *tmp_pt = (unsigned int *) 0xC03FF000;
        tmp_pt[pt_index] = phys | flags | PAGE_PRESENT;

        // Register new page table in page directory
        page_directory[pd_index] = new_pt_phys | flags | PAGE_PRESENT;    
        tlb_flush(virt);

        // remove temp mapping
        page_table[1023] = 0;
        tlb_flush(0xC03FF000);

        return 0;
    }

    // page table already exists, use temp mapping to access it
    unsigned int pt_phys = page_directory[pd_index] & ~0xFFF;
    page_table[1023] = pt_phys | PAGE_PRESENT | PAGE_RW;
    tlb_flush(0xC03FF000);

    unsigned int *tmp_pt = (unsigned int *)0xC03FF000;
    tmp_pt[pt_index] = phys | flags | PAGE_PRESENT;

    // remove temp mapping
    page_table[1023] = 0;
    tlb_flush(0xC03FF000);

    tlb_flush(virt);
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
    if (!(page_directory[pd_index] & PAGE_PRESENT)) return;

    // access page table by temp mapping
    unsigned int pt = page_directory[pd_index] & ~0xFFF;
    page_table[1023] = pt | PAGE_PRESENT | PAGE_RW;
    tlb_flush(0xC03FF000);

    unsigned int *tmp_pt = (unsigned int *) 0xC03FF000;
    tmp_pt[pt_index] = 0;

    page_table[1023] = 0;
    tlb_flush(0xC03FF000);
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

    // if page directory entry is empty -> return
    if (!(page_directory[pd_index] & PAGE_PRESENT)) return 0;

    // access page table by temp mapping
    unsigned int pt = page_directory[pd_index] & ~0xFFF;
    page_table[1023] = pt | PAGE_PRESENT | PAGE_RW;
    tlb_flush(0xC03FF000);

    // save the physical address for that PTE
    unsigned int *tmp_pt = (unsigned int *) 0xC03FF000;
    if (!(tmp_pt[pt_index] & PAGE_PRESENT)) {
        page_table[1023] = 0;
        tlb_flush(0xC03FF000);
        return 0;
    }
    unsigned int phys_addr = tmp_pt[pt_index] & ~0xFFF;

    // revert temp mapping
    page_table[1023] = 0;
    tlb_flush(0xC03FF000);

    return phys_addr;
}

/**
 * tlb_flush:
 *   Invalidates a single TLB entry for the given virtual address.
 *   Must be called after modifying a page table entry to ensure
 *   the CPU uses the updated mapping.
 *
 * @param virt  Virtual address whose TLB entry should be invalidated
 */
static inline void tlb_flush(unsigned int virt) {
    __asm__ volatile("invlpg (%0)" : : "r"(virt) : "memory");
}