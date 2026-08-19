#include "../../include/syscall_nums.h"
#include "ulib.h"
/* Userland side of the system call interface, plus the string helpers the
 * programs need. The string functions are plain computation and touch only
 * memory the process already owns, so they are NOT system calls -- the
 * kernel's kstrcmp/kstrlen are a separate copy that user code cannot link
 * against or even reach (kernel pages are mapped without PAGE_USER). */

/** syscall3:
 *  The only way into the kernel. The call number goes in eax and the three
 *  arguments in ebx/ecx/edx, matching what syscall_dispatch reads; eax
 *  carries the result back out.
 *
 *  @param num  System call number (see syscall_nums.h)
 *  @param a    First argument (ebx)
 *  @param b    Second argument (ecx)
 *  @param c    Third argument (edx)
 *  @return     Whatever the handler left in eax, -1 on error
 */
int syscall3(int num, int a, int b, int c) {
    int ret;
    __asm__ volatile ("int $0x80" : 
        "=a"(ret) : "a"(num), "b"(a), "c"(b), "d"(c) : "memory");
    return ret;
}

/** read:
 *  Reads up to count bytes into buf. On the keyboard this returns as soon
 *  as a single character is available and blocks when there is none, so a
 *  short read is normal rather than an error.
 *
 *  @param fd       Descriptor to read from
 *  @param buf      Destination buffer
 *  @param count    Maximum bytes to read
 *  @return         Bytes read (0 = end of file), -1 on error
 */
int read(int fd, void *buf, unsigned int count) {
    return syscall3(SYS_READ, fd, (int)(unsigned long) buf, count);
}

/** write:
 *  Writes n bytes from buf. The length is explicit and a NUL inside the
 *  buffer is written like any other byte -- terminators are a convention of
 *  the string functions, not of write.
 *
 *  @param fd   Descriptor to write to (1 = console)
 *  @param buf  Bytes to write
 *  @param n    How many bytes
 *  @return     Bytes written, -1 on error
 */
int write(int fd, const void *buf, unsigned n) {
    return syscall3(SYS_WRITE, fd, (int)(unsigned long) buf, n);
}

/** open:
 *  Looks a name up in the initrd and binds it to the lowest free descriptor.
 *  flags and mode are accepted but ignored -- every file opens read-only at
 *  offset 0.
 *
 *  @param filename     Name in the initrd (null-terminated)
 *  @param flags        Ignored
 *  @param mode         Ignored
 *  @return             The new descriptor, -1 if the file does not exist
 */
int open(const char *filename, int flags, int mode) {
    return syscall3(SYS_OPEN, (int)(unsigned long) filename, flags, mode);
}

/** close:
 *  Releases a descriptor so its number can be reused. The file itself is
 *  untouched; initrd files live in the tar image, not on a heap.
 *
 *  @param fd   Descriptor to release
 *  @return     0 on success, -1 if it was not open
 */
int close(int fd) {
    return syscall3(SYS_CLOSE, fd, 0, 0);
}

/** sleep:
 *  Suspends the process until the deadline passes; the CPU goes to another
 *  process meanwhile, so this costs no cycles.
 *
 *  @param seconds  How long to sleep
 *  @return         0
 */
int sleep(unsigned int seconds) {
    return syscall3(SYS_SLEEP, seconds, 0, 0);
}

/** strcmp:
 *  Compares two strings character by character. The loop stops at the first
 *  difference or at the end of s1, and the difference at that point answers
 *  both questions at once -- zero means every character matched.
 *
 *  @param s1   First null-terminated string
 *  @param s2   Second null-terminated string
 *  @return     0 if equal, negative if s1 sorts first, positive otherwise
 */
int strcmp(const char *s1, const char *s2) {
    while (*s1 != 0 && *s1 == *s2) { 
        s1++, s2++;
    }
    return (int) *s1 - *s2;   
}

/** strlen:
 *  Counts the characters before the terminator, which is not itself
 *  counted. Mostly used to hand write() a length for a string literal.
 *
 *  @param s    A null-terminated string
 *  @return     Its length in characters
 */
int strlen(const char *s) {
    const char * cur = s;
    int count = 0;
    while (*cur != 0) {
        cur++, count++;
    }
    return count;
}

int puts(char* s) {
    int n = write(SYSOUT_FILENO, s, strlen(s));
    return n;
}

/** exit:
 *  Terminates the process. The kernel drops it from the ready ring and
 *  switches away, so control never comes back -- the trailing loop only
 *  exists to satisfy the compiler if it ever did.
 *
 *  @param status   Exit status (logged by the kernel, nothing reads it yet)
 */
void exit(int status) {
    syscall3(SYS_EXIT, status, 0, 0);
    for (;;); // cannot go through
}
