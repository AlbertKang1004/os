# x86 OS Project

An OS written in C and NASM, booted with GRUB.
Targets x86 (32-bit protected mode) with a higher-half kernel at 0xC0100000.

---

## Features

| Component | Description | Date Added |
|-----------|-------------|------------|
| Framebuffer | Text output to screen | 2025-05-19 |
| GDT | Global Descriptor Table | 2026-05-21 |
| IDT | Interrupt Descriptor Table | 2026-05-22 |
| PIC | Programmable Interrupt Controller | 2026-05-22 |
| Keyboard | IRQ1 scancode handler | 2026-05-22 |
| Paging | Higher-half kernel, 4KB pages, PSE | 2026-05-27 |
| PMM | Physical memory manager (bitmap) | 2026-05-27 |
| VMM | Virtual memory manager, page mapping | 2026-06-07 |
| Heap | kmalloc/kfree, first-fit allocator | 2026-06-09 |
| Processes | process_create: per-process page directory, code/stack mapping | 2026-06-18 |
| User Mode | Ring 3 execution, TSS, user segments | 2026-06-26 |
| Syscalls | int 0x80 gate, dispatch table, user-side syscall lib | 2026-06-28 |
| PIT | Programmable Interval Timer, 100Hz tick | 2026-07-01 |
| Scheduler | Preemptive round-robin across user processes | 2026-07-07 |
| Interrupt Dispatch | Handler registration table, drivers self-register | 2026-07-09 |
| Exceptions | Page fault handler (cr2/eip dump, halt) | 2026-07-09 |

---

## Build

```bash
make run
```

Requires `nasm`, `gcc` (i686-elf cross-compiler), `genisoimage`, and `qemu-system-i386`.

---

## References

- [OSDev Wiki](https://wiki.osdev.org/Main_Page)
- [The Little Book About OS Development](https://littleosbook.github.io/)
- [x86 OS Tutorial (YouTube)](https://www.youtube.com/watch?v=yK1uBHPdp30)