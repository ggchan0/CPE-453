#ifndef LIBTINYFS_H
#define LIBTINYFS_H

#include "tinyFS.h"
#include "TinyFS_errno.h"

typedef struct freeBlock {
   int block_num;
   struct freeBlock *next;
} freeBlock;

typedef struct fileEntry {
   char *name;
   char *creation_time;
   char *modification_time;
   char *access_time;
   fileDescriptor fd;
   int inode_block_num;
   int num_copies;
   int permissions;
   int file_size;
} fileEntry;

typedef struct openFile {
   fileDescriptor fd;
   int file_index;
   int first_block;
   int cur_block;
   int cur_position;
} openFile;

int tfs_mkfs(char *filename, int nBytes);

int tfs_mount(char *filename);

int tfs_unmount(void);

fileDescriptor tfs_openFile(char *name);

int tfs_closeFile(fileDescriptor FD);

int tfs_writeFile(fileDescriptor FD, char *buffer, int size);

int tfs_deleteFile(fileDescriptor FD);

int tfs_readByte(fileDescriptor FD, char *buffer);

int tfs_seek(fileDescriptor FD, int offset);

int getNumFreeBlocks();

#endif
