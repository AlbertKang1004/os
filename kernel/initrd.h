#ifndef INITRD_H
#define INITRD_H

#define TAR_BLOCK_SIZE 512
#define TAR_NAME_SIZE 100

int tar_lookup(unsigned char *archive, char *filename, char **out);



#endif