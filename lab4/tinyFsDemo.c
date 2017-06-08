#include "libDisk.h"

int main(void) {
   int disk = openDisk("test.txt", BLOCKSIZE * 4);
   int write_status, read_status;
   char *write_block = calloc(1, BLOCKSIZE);
   char *read_block = calloc(1, BLOCKSIZE);
   write_block[0] = 'a';
   write_status = writeBlock(disk, 4, write_block);
   read_status = readBlock(disk, 2, read_block);
   printf("%s\n", read_block);
   printf("\n\n %d, %d\n", write_status, read_status);
   return 0;
}
