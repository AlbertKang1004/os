#ifndef MULTIBOOT_H
#define MULTIBOOT_H
#define KERNEL_VIRTUAL_BASE 0xC0000000

typedef struct multiboot_info {
    unsigned int flags;
    unsigned int mem_lower;
    unsigned int mem_upper;
    unsigned int boot_device;
    unsigned int cmdline;
    unsigned int mods_count;
    unsigned int mods_addr;

    unsigned int syms[4];
    unsigned int mmap_length;
    unsigned int mmap_addr;
    unsigned int drives_length;
    unsigned int drives_addr;
    unsigned int config_table;
    unsigned int boot_loader_name;
} __attribute__((packed)) multiboot_info_t;

typedef struct {
    unsigned int mod_start;
    unsigned int mod_end;
    unsigned int string;
    unsigned int reserved;
} __attribute__((packed)) multiboot_module_t;

void multiboot_relocate(multiboot_info_t *mbinfo);

#endif