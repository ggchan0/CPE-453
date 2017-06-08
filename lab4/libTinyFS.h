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

void *checkedCalloc(int size);

int writeSuperblock();

freeBlock *addFreeBlock(freeBlock *tail, int block_num);

int popFreeBlock();

int setupFreeBlocks();

int getFileSize(char *inode_buffer);

void getFileTimes(fileEntry *file, char *inode_buffer);

int loadFiles();

int getNumFreeBlocks();

freeBlock *getTail();

int loadAllData();

void updateINode(fileEntry *file);

int saveAllData();

char *getCurrentTime();

void shiftOpenFileTable(int index);

void shiftFileTable(int index);

int findFileEntry(char* filename);

int getOpenFile(fileDescriptor fd);

int getFileEntry(fileDescriptor fd);

char *initINode(openFile *file, fileEntry *file_entry, int size);

void printBlockChain(int start_block);

int tfs_rename(char* old_name, char* new_name);

int tfs_makeRO(char *filename);

int tfs_makeRW(char *filename);

int tfs_writeByte(fileDescriptor fd, int offset, unsigned char data);

int tfs_readFileInfo(int fd);

int tfs_readdir();



#endif
