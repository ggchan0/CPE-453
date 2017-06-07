#ifndef TINYFS_H
#define TINYFS_H

#define DEFAULT_DISK_NAME "tinyFSDisk"
#define DEFAULT_DISK_SIZE 10240
#define DEFAULT_BLOCK_AMOUNT 40
#define BLOCKSIZE 256

//block types
#define EMPTY 0
#define SUPERBLOCK 1
#define INODE 2
#define FILE_EXTENT 3
#define FREE_BLOCK 4

//magic number
#define MAGIC_NUMBER 0x45

//block metadata by the byte
#define TYPE_BYTE 0
#define MAGIC_BYTE 1
#define ADDR_BYTE 2
#define FS_SIZE_BYTE 3
#define SIZE_BYTE 4

//permission bits
#define READ_ONLY 0
#define READ_WRITE 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>

typedef int fileDescriptor;

#endif
