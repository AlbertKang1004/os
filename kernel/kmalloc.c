#include "kmalloc.h"
#include "pmm.h"
#include "vmm.h"

#define HEAP_START 0xC0400000

static struct block_header *heap_start = 0;
static unsigned int heap_next = HEAP_START; // tracking the next address for the new page

/**
 * kmalloc:
 *   Allocates a block of memory of at least size bytes from the kernel heap.
 *   If no free block is large enough, allocates a new page frame.
 *
 * @param size      Number of bytes to allocate
 * @return          Pointer to allocated memory, or 0 on failure
 */
void *kmalloc(unsigned int size) {
    if (heap_start == 0) {
        // first call, initialize heap
        unsigned int phys = pmm_alloc();
        if (phys == 0) return 0;
        if (vmm_map_page(phys, HEAP_START, PAGE_PRESENT | PAGE_RW) < 0) return 0;
        heap_start = (struct block_header *)HEAP_START;
        heap_start->size = PAGE_SIZE - sizeof(struct block_header);
        heap_start->free = 1;
        heap_start->next = 0;
    }

    struct block_header* cur = heap_start;
    struct block_header* prev = 0;

    // find first available space
    while (cur != 0){
        if (cur->free && cur->size >= size) {
            struct block_header *next_block = (struct block_header*)((unsigned char *)cur + sizeof(struct block_header) + size);
            next_block->size = cur->size - sizeof(struct block_header) - size;
            next_block->free = 1; 
            next_block->next = 0;

            cur->size = size;
            cur->free = 0;
            cur->next = next_block;

            return (void *)((unsigned char *)cur + sizeof(struct block_header));
        }
        prev = cur;
        cur = cur->next;
    }

    // no space available in current block
    unsigned int new_phys = pmm_alloc();
    if (new_phys == 0) return 0;
    if (vmm_map_page(new_phys, heap_next, PAGE_PRESENT | PAGE_RW) < 0) return 0;
    cur = (struct block_header *)(unsigned long) heap_next;
    cur->size = size;
    cur->free = 0;

    struct block_header *next_block = (struct block_header *)((unsigned char *)cur + sizeof(struct block_header) + size);
    next_block->size = PAGE_SIZE - 2 * sizeof(struct block_header) - size;
    next_block->free = 1;
    next_block->next = 0;
    cur->next = next_block;
    prev->next = cur;
    heap_next += PAGE_SIZE;
    return (void *)((unsigned char *)cur + sizeof(struct block_header));
}

/**
 * kfree:
 *   Frees a previously allocated block of memory.
 *   If the freed block is large enough, returns the page frame to pmm.
 *
 * @param ptr       Pointer to memory block to free
 */
void kfree(void *ptr) {
    if (ptr == 0) return;
    struct block_header *cur = (struct block_header *)((unsigned char *)ptr - sizeof(struct block_header));
    if (cur->next != 0 && cur->next->free) { // if next block exists and next block is free
        cur->size += cur->next->size + sizeof(struct block_header);
        cur->next = cur->next->next;
    }
    cur->free = 1;
}