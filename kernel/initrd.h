#ifndef INITRD_H
#define INITRD_H

#define TAR_BLOCK_SIZE 512
#define TAR_NAME_SIZE 100

void initrd_init(unsigned char *base);
int tar_lookup(const char *filename, char **out);

#endif