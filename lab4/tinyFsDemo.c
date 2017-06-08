#include "libTinyFS.h"

void reset() {
   tfs_mkfs("test.txt", 4 * BLOCKSIZE);
   tfs_unmount();
   exit(0);
}

int main(void) {
   //reset();
   //int disk = tfs_mkfs("test.txt", 4 * BLOCKSIZE);
   int disk = tfs_mount("test.txt");
   char *filename = "hi.txt";
   //tfs_unmount();
   printf("disk no %d\n", disk);
   printf("free blocks %d\n", getNumFreeBlocks());

   int fd = tfs_openFile(filename);
   printf("fd %d\n", fd);

   //int delete_status = tfs_deleteFile(fd);
   //printf("delete_status %d\n", delete_status);

   int write_status = tfs_writeFile(fd, "hello", 5);
   printf("write_status %d\n", write_status);

   //int close_status = tfs_closeFile(fd);
   //printf("close_status %d\n", close_status);
   tfs_unmount();
   return 0;
}
