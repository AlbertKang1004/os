#ifndef KMALLOC_H
#define KMALLOC_H

struct block_header {
    unsigned int size;         // size of data block (bytes)
    unsigned int free;         // 1 = free, 0 = used
    struct block_header *next; // next block in free list
} __attribute__((packed));

void *kmalloc(unsigned int size);
void kfree(void *ptr);

#endif