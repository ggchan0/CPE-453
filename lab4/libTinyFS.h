#ifndef LIBTINYFS_H
#define LIBTINYFS_H

#include "tinyFS.h"

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

#endif
