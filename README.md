# x86 OS Project

An OS written in C and NASM, booted with GRUB.
Targets x86 (32-bit protected mode) with a higher-half kernel at 0xC0100000.

---

## Features

| Component | Description | Date Added |
|-----------|-------------|------------|
| Framebuffer | Text output to screen | 2026-05-19 |
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
| PIT | Programmable Interval Timer, 1000Hz tick (1ms), 100-tick quantum | 2026-07-01 |
| Scheduler | Preemptive round-robin across user processes | 2026-07-07 |
| Interrupt Dispatch | Handler registration table, drivers self-register | 2026-07-09 |
| Exceptions | Page fault handler (cr2/eip dump, halt) | 2026-07-09 |
| sys_exit | Process termination: ready-ring removal, switch via trap frame rewrite | 2026-07-10 |
| sys_sleep | Tick-based sleep (SLEEPING state + wake_tick), idle process fallback | 2026-07-16 |
| Initrd | USTAR tar parser (tar_lookup), named programs loaded from a single module | 2026-07-22 |
| File Descriptors | Per-process fd table, type-tagged (file/serial/keyboard), stdin/stdout/stderr preopened | 2026-07-24 |
| File I/O | open/read/close on initrd files, write dispatched by fd type | 2026-07-24 |
| Keyboard Decoder | Scancode set 1 state machine: make/break, 0xE0 prefix, shift/ctrl/alt/caps | 2026-08-04 |
| Input Buffer | 128-byte ring buffer, free-running indices, drops on overflow | 2026-08-04 |
| Blocking Read | BLOCKED state, syscall restart via eip rewind, wake from IRQ1 | 2026-08-04 |
| Wait Queues | Generic block/wake list, driver decoupled from the scheduler | 2026-08-06 |
| Console | Framebuffer driver: cursor, control characters, scrolling; sys_write goes to screen and serial | 2026-08-07 |

---

## Project Structure

Legend: unmarked = done &nbsp;·&nbsp; `+` added most recently &nbsp;·&nbsp; `!` incomplete &nbsp;·&nbsp; `@@` currently working on &nbsp;·&nbsp; `-` not written yet

```diff
  OS/
  ├── boot/
  │   └── loader.s                  # multiboot header, kernel entry point
  ├── drivers/
+ │   ├── fb.c/.h                   # VGA text console: cursor, control chars, scrolling
  │   ├── keyboard.c/.h             # scancode decoder + input ring buffer
  │   ├── pic.c/.h                  # 8259 PIC remap
  │   ├── pit.c/.h                  # 1000 Hz timer (1 tick = 1ms)
  │   └── serial.c/.h               # COM1 output
  ├── include/
  │   └── syscall_nums.h            # syscall numbers shared by kernel and userland
  ├── kernel/
! │   ├── kmain.c                   # boot entry, subsystem init, first process
  │   ├── multiboot.c/.h            # GRUB info structures
  │   ├── gdt.c/.h                  # segmentation incl. user segments
  │   ├── tss.c/.h                  # task state segment (ring 3 -> ring 0 stack)
  │   ├── interrupt.c/.h            # handler registration table + dispatch
  │   ├── interrupt_asm.s           # entry stubs
  │   ├── interrupt_handlers.*      # generated IDT entries
  │   ├── exceptions.c/.h           # page fault handler (cr2/eip dump)
  │   ├── pmm.c/.h                  # physical memory manager (bitmap)
  │   ├── vmm.c/.h                  # virtual memory manager (recursive mapping)
  │   ├── kmalloc.c/.h              # first-fit kernel heap
  │   ├── process.c/.h              # process_create, PCB, fd table
! │   ├── scheduler.c/.h            # round-robin ring, sleep/exit/wake - no kernel<->user switch yet
+ │   ├── wait.c/.h                 # wait queues: block/wake without touching the scheduler
  │   ├── syscall.c/.h              # int 0x80 dispatch, open/read/write/close, blocking read
  │   ├── initrd.c/.h               # USTAR tar parser (tar_lookup)
  │   ├── usermode.s                # ring 3 entry
  │   ├── utils.c/.h                # kmemcpy, kstrcmp, print_hex
  │   ├── cpu.h                     # cr2/cr3, cli/hlt inline helpers
  │   └── debug.h                   # LOG / LOG_HEX / LOG_STR macros
  ├── lib/
  │   └── io.h/.s                   # port I/O
  ├── iso/modules/                  # userland - packed into initrd.tar
  │   ├── start.s                   # user program crt0
  │   ├── ulib.c/.h                 # syscall wrappers (open/read/write/exit/sleep)
  │   ├── prog_a.c                  # test program
  │   ├── prog_b.c                  # test program
  │   ├── idle.c                    # idle loop
  │   ├── hello.txt                 # sample data file read through the fd layer
@@│   └── shell.c                   # NEXT - prompt, ls, cat, run programs by name @@
  ├── generate_idt.py               # emits interrupt_handlers.*
  ├── link.ld                       # kernel linker script (higher half)
  ├── user.ld                       # user program linker script
  ├── Makefile
  └── README.md
```

Build artifacts (`*.o`, `kernel.elf`, `os.iso`, `com1.out`, `initrd.tar`) are generated by `make` and gitignored.

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