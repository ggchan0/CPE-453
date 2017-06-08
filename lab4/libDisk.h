#ifndef LIBDISK_H
#define LIBDISK_H

#include "tinyFS.h"

int fileSize(int disk);

int openDisk(char *filename, int nBytes);

int readBlock(int disk, int bNum, void *block);

int writeBlock(int disk, int bNum, void *block);

void closeDisk(int disk);

#endif
