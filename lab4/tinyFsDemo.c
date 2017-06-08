#include "libTinyFS.h"

void reset() {
   tfs_mkfs("test.txt", 8 * BLOCKSIZE);
   tfs_unmount();
   exit(0);
}

int main(void) {
   int disk = tfs_mkfs("test.txt", DEFAULT_DISK_SIZE);
   tfs_mount("test.txt");
   printf("Disk number: %d\n", disk);
   printf("Free blocks: %d\n\n", getNumFreeBlocks());

   printf("Opening file: foo.txt\n");
   int foo = tfs_openFile("foo.txt");
   printf("Foo.txt's fd: %d\n", foo);
   tfs_readFileInfo(foo);

   printf("Free blocks: %d\n\n", getNumFreeBlocks());
   printf("Files on the system: ");
   tfs_readdir();

   printf("Closing foo.txt\n");
   int close_file_status = tfs_closeFile(foo);
   printf("Close file code: %d\n", close_file_status);

   printf("Free blocks: %d\n\n", getNumFreeBlocks());

   printf("Opening file: bar.txt\n");
   int bar = tfs_openFile("bar.txt");
   printf("Bar.txt's fd: %d\n", bar);
   printf("Writing 'hello' to bar.txt\n");
   int write_file_status = tfs_writeFile(bar, "hello", 5);
   printf("Write file code: %d\n", write_file_status);
   printf("Free blocks: %d\n\n", getNumFreeBlocks());

   char buf[1];
   printf("Reading first bit of bar (h)\n");
   printf("Readbyte status code: %d\n", tfs_readByte(bar, buf));
   printf("Character read: %c\n", buf[0]);

   printf("Seeking to fourth character in bar (o)\n");
   printf("Seek status code: %d\n", tfs_seek(bar, 4));
   printf("Readbyte status code: %d\n", tfs_readByte(bar, buf));
   printf("Character read: %c\n", buf[0]);

   printf("\nMaking bar read only\n");
   tfs_makeRO("bar.txt");
   printf("Writing byte to read only bar.txt status code: %d\n\n", tfs_writeByte(bar, 2, '!'));

   printf("Files on the system: ");
   tfs_readdir();

   tfs_readFileInfo(bar);
   printf("Renaming bar.txt to baz.txt\n");
   tfs_rename("bar.txt", "baz.txt");
   tfs_readFileInfo(bar);

   printf("\nMaking baz read-write\n");
   tfs_makeRW("baz.txt");
   printf("Files on the system: ");
   tfs_readdir();
   printf("Deleting baz.txt\n");
   printf("Deletion status code %d\n", tfs_deleteFile(bar));
   printf("Files on the system: ");
   tfs_readdir();
   return 0;
}

// int main(void) {
//    //reset();
//    //int disk = tfs_mkfs("test.txt", 4 * BLOCKSIZE);
//    int disk = tfs_mount("test.txt");
//    char *filename = "hi.txt";
//    //tfs_unmount();
//    printf("disk no %d\n", disk);
//    printf("free blocks %d\n", getNumFreeBlocks());
//
//    int fd = tfs_openFile(filename);
//    printf("fd %d\n", fd);
//
//    //int delete_status = tfs_deleteFile(fd);
//    //printf("delete_status %d\n", delete_status);
//
//    tfs_readdir();
//
//    char buf[300];
//    for (int i = 0; i < 300; i++) {
//       buf[i] = 'a';
//    }
//
//    int write_status = tfs_writeFile(fd, buf, 300);
//    printf("write_status %d\n", write_status);
//
//    printf("%s\n", getCurrentTime());
//
//    //
//    // for (int i = 0; i < 5; i++) {
//    //    char buf[1];
//    //    if (tfs_readByte(fd, buf) >= 0) {
//    //       printf("%c\n", buf[0]);
//    //    }
//    //
//    // }
//
//    //int close_status = tfs_closeFile(fd);
//    //printf("close_status %d\n", close_status);
//    tfs_unmount();
//    return 0;
// }
