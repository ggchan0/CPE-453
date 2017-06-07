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
//stores first free block for super block
//stores first data block for inode
//stores next data block for data block/file extent
//stores next free block for a free block
#define ADDR_BYTE 2

//for the superblock
#define FS_SIZE_BYTE 3
#define SIZE_BYTE 4

//for the inode
#define PERMISSION_BYTE 6
#define NAME_BYTE 8
#define TIMESTAMP_SIZE 26
#define CREATE_TIME_BYTE 20
#define MOD_TIME_BYTE 46
#define ACCESS_TIME_BYTE 72

//timestamp bits
#define CREATION 1
#define MODIFICATION 2
#define ACCESS 4

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
