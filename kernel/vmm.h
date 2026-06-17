#ifndef VMM_H
#define VMM_H

#define PAGE_PRESENT    0x01
#define PAGE_RW         0x02
#define PAGE_USER       0x04
#define PAGE_KERNEL     0x00

#define PD_VIRT     ((unsigned int *) 0xFFFFF000)
#define PT_VIRT(n)  ((unsigned int *)(0xFFC00000 + (n) * 0x1000))


int vmm_map_page(unsigned int phys, unsigned int virt, unsigned int flags);
void vmm_unmap_page(unsigned int virt);
unsigned int vmm_get_phys(unsigned int virt);
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
static int pt_is_empty(unsigned int pd_index);
#endif