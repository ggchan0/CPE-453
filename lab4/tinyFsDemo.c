#include "libTinyFS.h"

int main(void) {
   //int disk = tfs_mkfs("test.txt", 4 * BLOCKSIZE);
   int disk = tfs_mount("test.txt");
   //tfs_unmount();
   printf("%d\n", disk);
   printf("%d\n", getNumFreeBlocks());
   return 0;
}
