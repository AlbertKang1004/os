#include "initrd.h"
#include "utils.h"
#include "debug.h"

struct tar_header
{
    char filename[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag[1];
};

static unsigned char *archive_base;

void initrd_init(unsigned char *base) { // setter
    archive_base = base;
}

unsigned char * get_archive_base(void) { // getter
    return archive_base;
}

/**
 * oct2bin:
 *   Converts a fixed-length octal ASCII string to an integer.
 *   (Tar stores numeric fields, such as file size, as octal text.)
 *
 * @param str    Pointer to the octal digits (not null-terminated; length is given)
 * @param size   Number of characters to read from str
 * @return       The decoded integer value
 */
static int oct2bin(unsigned char *str, int size) {
    int n = 0;
    unsigned char *c = str;
    for (int i = 0; i < size; i++) {
        if (str[i] < '0' || str[i] > '7') break;
        n *= 8;
        n += *c - '0';
        c++;
    }
    return n;
}

/**
 * tar_lookup:
 *   Searches a USTAR archive for a file by name. Walks the archive header
 *   by header until the entry is found or the end of the archive is reached.
 *
 * @param archive   Base address of the tar archive in memory
 * @param filename  Name of the file to find
 * @param out       On success, set to the address of the file's data
 * @return          The size of the file in bytes if found; -1 if not found
 */
int tar_lookup(const char *filename, char **out) {
    unsigned char * current = archive_base;
    while (kstrcmp((char *) (current + 257), "ustar") == 0) {
        int file_size = oct2bin(current + 124, 12);
        if (kstrncmp((char *) current, filename, 100) == 0) {
            *out = (char *) current + TAR_BLOCK_SIZE;
            return file_size;
        }
        // go to next entry
        current += (TAR_BLOCK_SIZE + ((TAR_BLOCK_SIZE - 1 + file_size) / TAR_BLOCK_SIZE) * TAR_BLOCK_SIZE);
    }
    return -1;
}