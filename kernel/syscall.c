#include "syscall.h"
#include "interrupt.h"
#include "../drivers/keyboard.h"
#include "../drivers/pit.h"
#include "../lib/io.h"
#include "../drivers/serial.h"
#include "../drivers/fb.h"
#include "../include/syscall_nums.h"
#include "scheduler.h"
#include "debug.h"
#include "initrd.h"
#include "process.h"
#include "kmalloc.h"

extern unsigned int multiboot_info_ptr;

/* System call layer. User code enters through `int 0x80` with the call
 * number in eax and arguments in ebx/ecx/edx; the result goes back in eax.
 * Every handler therefore takes the same trap frame rather than named
 * arguments, and pulls its parameters out of `cpu`. Pointer arguments
 * arrive as integers and are cast back -- they are NOT yet validated as
 * pointing into user space. */

syscall_handler_t syscall_table[SYSCALL_TABLE_SIZE] = {
    [0] = sys_read,
    [1] = sys_write,
    [2] = sys_open,
    [3] = sys_close,
    [4] = sys_exit,
    [5] = sys_nanosleep,
    [6] = sys_readdir
};

/** sys_read:
 *  Reads up to `count` bytes into the user buffer. What that means
 *  depends on the descriptor type: tar files copy from the current
 *  offset and advance it, serial is write-only.
 *
 *  @param cpu  ebx = fd, ecx = user buffer, edx = byte count.
 *  @return     Bytes read (0 = end of file), or -1 for a bad fd or a
 *              type that cannot be read.
 */
int sys_read(struct cpu_state * cpu, struct stack_state * stack) {
    unsigned int fd = cpu->ebx;
    char *buf = (char *)(unsigned long) cpu->ecx;
    unsigned int count = cpu->edx;

    struct process *proc = scheduler_current();
    if (fd >= FD_MAX || proc->fd_table[fd] == 0) return -1;
    switch (proc->fd_table[fd]->type) {
        case FD_SERIAL: return -1;
        case FD_KEYBOARD: {
            int c;
            unsigned int size = 0;
            while (size < count) {
                c = keyboard_read();
                if (c == -1) break;

                *buf = (char) c;
                buf++, size++;
            }
            if (size == 0) { // block the process

                keyboard_wait(cpu, stack);
            }
            return size;
        }
        case FD_TAR_FILE: {
            struct fd * current_fd = proc->fd_table[fd];
            unsigned char * pt = (unsigned char *)(unsigned long) current_fd->data + current_fd->offset;
            if (count > current_fd->size - current_fd->offset)
                count = current_fd->size - current_fd->offset;
            kmemcpy(buf, pt, count);
            current_fd->offset += count;
            return count;
        }
        case FD_NONE: return -1;
        default: return -1;
    }
}

/** sys_write:
 *  Writes `count` bytes from the user buffer. 
 *  tar files live in the initrd image and are read-only.
 *
 *  @param cpu  ebx = fd, ecx = user buffer, edx = byte count.
 *  @return     Bytes written, or -1 for a bad fd or a read-only type.
 */
int sys_write(struct cpu_state * cpu, struct stack_state * stack) {
    (void) stack;
    unsigned int fd = cpu->ebx;
    const char *buf = (const char *)(unsigned long) cpu->ecx;
    unsigned int count = cpu->edx;

    struct process *proc = scheduler_current();
    if (fd >= FD_MAX || proc->fd_table[fd] == 0) return -1;
    switch (proc->fd_table[fd]->type) {
        case FD_SERIAL:
            for (unsigned int i = 0; i < count; i++) {
                outb(SERIAL_COM1_BASE, buf[i]);
            }
            fb_write(buf, count);
            return count;
        case FD_KEYBOARD: 
        case FD_TAR_FILE: 
        case FD_NONE: 
        default: return -1;
    }
}

/** sys_open:
 *  Looks a name up in the tar initrd and binds it to the lowest free
 *  slot of the calling process's fd table. The descriptor is heap
 *  allocated and released by sys_close. flags and mode are accepted but
 *  not yet honoured -- every file opens read-only at offset 0.
 *
 *  @param cpu  ebx = filename (NUL-terminated, user pointer),
 *              ecx = flags, edx = mode.
 *  @return     The new fd, or -1 if the file does not exist, the table
 *              is full, or the allocation failed.
 */
int sys_open(struct cpu_state * cpu, struct stack_state * stack) {
    (void) stack;
    const char *filename = (const char *)(unsigned long) cpu->ebx;
    int flags = cpu->ecx;
    int mode = cpu->edx;
    char *out;
    int file_size = tar_lookup(filename, &out);
    if (file_size == -1) { return -1; }
    struct process *proc = scheduler_current();
    int i;
    for (i = 0; i < FD_MAX && proc->fd_table[i] != 0; i++);
    if (i >= FD_MAX) return -1;

    if ((proc->fd_table[i] = kmalloc(sizeof(*proc->fd_table[i]))) == 0) {
        return -1; // failed to allocate space
    }

    struct fd * current_fd = proc->fd_table[i];
    current_fd->type = FD_TAR_FILE;
    current_fd->data = (unsigned int)(unsigned long) out;
    current_fd->size = file_size;
    current_fd->offset = 0;
    current_fd->flags = 0;

    return i;
}

/** sys_close:
 *  Releases a descriptor: frees the struct fd and clears the table slot
 *  so the number can be reused. The underlying data is untouched -- tar
 *  files live in the initrd image, not on the heap.
 *
 *  @param cpu  ebx = fd.
 *  @return     0 on success, -1 if the fd was out of range or not open.
 */
int sys_close(struct cpu_state * cpu, struct stack_state * stack) {
    (void) stack;
    unsigned int fd = cpu->ebx;
    struct process *proc = scheduler_current();

    if (fd >= FD_MAX || proc->fd_table[fd] == 0) return -1;
    kfree(proc->fd_table[fd]);
    proc->fd_table[fd] = 0;
    
    return 0;
}

/** sys_readdir:
 *  Copies the name of the index-th entry of the initrd into a user buffer.
 *  There are no directories yet, so the archive itself is the only thing
 *  that can be listed and no path argument is needed. The name is COPIED
 *  rather than returned by pointer: the archive lives above 0xC0000000 in
 *  pages mapped without PAGE_USER, so a kernel pointer would be unusable
 *  and unsafe in ring 3.
 *
 *  Walking from the start on every call is O(n) per entry, which is the
 *  price of keeping the call stateless -- an iterator would need somewhere
 *  per-process to live, which is exactly why opendir returns a descriptor.
 *
 *  @param cpu  ebx = entry index, ecx = user buffer, edx = buffer size.
 *  @return     Length of the name copied, or -1 once the index is past the
 *              last entry (which is how a caller knows to stop).
 */
int sys_readdir(struct cpu_state * cpu, struct stack_state * stack) {
    
}

/** sys_exit:
 *  Terminates the calling process. scheduler_exit_current drops it from
 *  the ready ring and overwrites the trap frame with the next process's
 *  saved state, so this never returns to its caller -- the pending iret
 *  resumes somebody else. The C return value is unused; see
 *  syscall_dispatch.
 *
 *  @param cpu  ebx = exit status (logged only).
 */
int sys_exit(struct cpu_state * cpu, struct stack_state * stack) {
    int status = cpu->ebx;
    LOG_HEX("process exited, status", status);
    scheduler_exit_current(cpu, stack);
    return 0; // not used anyways
}

/** sys_nanosleep:
 *  Puts the calling process to sleep, then lets the scheduler pick
 *  someone else; it resumes once the deadline passes. Like sys_exit this
 *  rewrites the trap frame, which is why eax is set HERE -- once
 *  scheduler_sleep_current has saved the frame into the PCB, a later
 *  assignment would land on the next process instead.
 *
 *  @param cpu  ebx = seconds to sleep (converted to 1 ms PIT ticks).
 */
int sys_nanosleep(struct cpu_state * cpu, struct stack_state * stack) {
    unsigned int tick = cpu->ebx * 1000;
    cpu->eax = 0;
    scheduler_sleep_current(cpu, stack, tick);
    return 0;
}

/** syscall_dispatch:
 *  The int 0x80 handler. Looks the number in eax up in syscall_table and
 *  stores the handler's return value back into eax.
 *
 *  A handler that switches away (exit, sleep, a blocking read) leaves
 *  `cpu` describing a DIFFERENT process, so writing eax afterwards would
 *  corrupt that process's registers. Rather than listing which calls do
 *  that, compare the current process across the call: if it changed, the
 *  frame is no longer ours and eax is left alone. `before` may dangle
 *  after sys_exit frees the PCB -- it is only ever compared, never
 *  dereferenced.
 */
static void syscall_dispatch(struct cpu_state * cpu, struct stack_state * stack, unsigned int interrupt) {
    (void) interrupt;

    unsigned int syscall_num = cpu->eax;
    if (syscall_num >= SYSCALL_TABLE_SIZE || syscall_table[syscall_num] == 0) { 
        cpu->eax = -1;
        return;
    }
    struct process *before = scheduler_current();
    int ret = syscall_table[syscall_num](cpu, stack);
    if (scheduler_current() == before) {
        cpu->eax = ret;
    }
}

/** syscall_init:
 *  Registers syscall_dispatch on interrupt 0x80. Must run before the
 *  first user process starts.
 */
void syscall_init() {
    register_interrupt_handler(0x80, syscall_dispatch);
}