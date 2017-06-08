#include "libDisk.h"

int fileSize(int disk) {
   return lseek(disk, 0, SEEK_END);
}

int openDisk(char *filename, int nBytes) {
   int file = -1;
   int i = 0;
   if (filename == NULL || nBytes < 0) {
      file = -1;
   } else if (nBytes == 0) {
      file = open(filename, O_RDWR);
   } else {
      file = open(filename, O_RDWR | O_TRUNC
            | O_CREAT, S_IRWXU, S_IRWXG,S_IRWXO);
      for (i = 0; i < nBytes; i++) {
         if (write(file, "\0", 1) == -1) {
            file = -1;
         }
      }
   }
   return file;
}

int readBlock(int disk, int bNum, void *block) {
   int status = 0;
   //check if disk is valid
   if (fcntl(disk, F_GETFD) == -1 || bNum < 0) {
      status = -1;
   //check if buffer exists
   } else if (block == NULL) {
      status = -1;
   //check if lseek worked
   } else if (bNum * BLOCKSIZE > fileSize(disk) - 256) {
      status = -1;
   } else {
      lseek(disk, bNum * BLOCKSIZE, SEEK_SET);
      read(disk, block, BLOCKSIZE);
   }


   return status;
}

int writeBlock(int disk, int bNum, void *block) {
   int status = 0;
   //check if disk is valid
   if (fcntl(disk, F_GETFD) == -1 || bNum < 0) {
      status = -1;
   //check if buffer exists
   } else if (block == NULL) {
      status = -1;
   //check if lseek worked
   } else if (bNum * BLOCKSIZE > fileSize(disk) - 256) {
      status = -1;
   } else {
      lseek(disk, bNum * BLOCKSIZE, SEEK_SET);
      write(disk, block, BLOCKSIZE);
   }

   return status;
}

void closeDisk(int disk) {
   if (fcntl(disk, F_GETFD) != -1) {
      close(disk);
   }
}
