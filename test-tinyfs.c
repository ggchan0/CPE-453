#include <stdio.h>
#define BLOCKSIZE 256
#define DEFAULT_DISK_SIZE 10240
#define DISKSIZE 4194304 //4MB
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

int demoStatus = 0;

void testMakeFsBadSize() {
  char file_name[] = "badFS";
  int status = 0;

  status = tfs_mkfs(file_name, 0);
  if (status < 0) {
    printf("Didn't make bad FS, test passes\n");
  } else {
    printf("Made FS with bad size, test fails\n");
    //exit(-1);
  }
}

void testMakeFsGood() {
  char file_name[] = "newFS";
  int status = 0;

  status = tfs_mkfs(file_name, DEFAULT_DISK_SIZE);
  if (status < 0) {
    printf("Didn't make good FS, test fails\n");
    demoStatus = 1;
    //exit(-1);
  } else {
    printf("Made FS with good size, test passes\n");
  }
}

void testMount() {
  char file_name[] = "newFS";
  int status = 0;

  status = tfs_mount(file_name);
  if (status < 0) {
    printf("Didn't mount good FS, test fails\n");
    demoStatus = 1;
    //exit(-1);
  } else {
    printf("Mounted good FS, test passes\n");
  }
}

int testOpenFile1() {
  int fd = 0;

  printf("Opening file new file new.txt\n");
  fd = tfs_openFile("new.txt");

  if (fd < 0) {
    printf("Did not get a good FD, test fails\n");
    printf("Cannot continue other tests since this test failed\n");
    demoStatus = 1;
    exit(-1);
  } else {
    printf("FD returned: %d, test passes\n", fd);
  }

  return fd;
}

void testReadWriteFile1(int fd, char *buf) {
  int status = 0;
  int len_read = 0;
  char read_buf[301];
  int i = 0;

  printf("Writing a short file of size %d to new.txt\n", (int) strlen(buf));
  status = tfs_writeFile(fd, buf, strlen(buf));

  if (status < 0) {
    printf("Error writing file, fails test\n");
    demoStatus = 1;
    return;
    //exit(-1);
  }

  tfs_seek(fd, 0);

  printf("Reading file back to verify contents\n");
  for (i = 0; tfs_readByte(fd, read_buf + i) >= 0; i++) {}

  read_buf[i] = '\0';

  printf("New.txt contents: %s\n", read_buf);
  printf("original contents: %s\n", buf);

  if (strcmp(read_buf, buf) == 0) {
    printf("Verified contents, test passes\n");
  } else {
    printf("Contents do not match, test fails\n");
    demoStatus = 1;
    //exit(-1);
  }
}

void testCloseFile(int fd) {
  int status = 0;
  int len_read = 0;
  char read_buf[1];

  printf("Closing fd %d\n", fd);
  status = tfs_closeFile(fd);
  if (status < 0) {
    printf("Error closing file for fd %d, test fails\n", fd);
    demoStatus = 1;
    //exit(-1)
  } else {
    printf("Closed file properly, test passes\n");
  }
}

void testDeleteFile(int fd) {
  int status = 0;
  char buf[1];

  printf("Deleting new.txt, which is fd %d\n", fd);
  status = tfs_deleteFile(fd);
  if (status < 0) {
    printf("Error code %d, didn't delete file properly, test fails\n", status);
    demoStatus = 1;
    return;
  } else {
    printf("Deleted file properly, verifying deletion\n");
  }

  fd = tfs_openFile("new.txt");
  status = tfs_readByte(fd, buf);
  if (status < 0) {
    printf("Couldn't read contents of a new file after it was deleted, test passes\n");
  } else {
    printf("Somehow read contents of empty file, test fails\n");
    demoStatus = 1;
  }
}

void testReadWriteFile2(int fd, char *buf) {
  int status = 0;
  int len_read = 0;
  char read_buf[301];
  int i = 0;

  printf("Writing a long file of size %d to n2.txt\n", (int) strlen(buf));
  status = tfs_writeFile(fd, buf, strlen(buf));

  if (status < 0) {
    printf("Error writing file, fails test\n");
    return;
    //exit(-1);
  }

  tfs_seek(fd, 0);

  printf("Reading file back to verify contents\n");
  for (i = 0; tfs_readByte(fd, read_buf + i) >= 0; i++) {}

  read_buf[i] = '\0';

  printf("n2.txt contents: %s\n", read_buf);
  printf("original contents: %s\n", buf);

  if (strcmp(read_buf, buf) == 0) {
    printf("Verified contents, test passes\n");
  } else {
    printf("Contents do not match, test fails\n");
    demoStatus = 1;
    //exit(-1);
  }
}

void testUnmountAndRemount() {
  int status = 0;

  status = tfs_unmount();
  if (status < 0) {
    printf("Unmount failed, test fails\n");
    demoStatus = 1;
    return;
  }

  status = tfs_mount("newFS");
  if (status < 0) {
    printf("Cannot remount, test fails\n");
    demoStatus = 1;
    return;
  } else {
    printf("Able to unmount and then remount, test passes\n");
  }
}

void testFilesExist() {
  int status = 0;
  char buf[1];
  int fd = 0;

  fd = tfs_openFile("n2.txt");
  status = tfs_readByte(fd, buf);
  if (status < 0) {
    printf("Unable to read file contents after remounting, test fails\n");
    demoStatus = 1;
    return;
  }

  if (*buf == '1') {
    printf("File is successfully written to disk after unmount, test passes\n");
  } else {
    printf("File contents are corrupted, test fails\n");
    demoStatus = 1;
  }
}

int main(){
  char short_file[] = "hello there!";
  char long_file[301];
  char file_name[] = "newFS";

  int disk_num = 0;
  int io_status = -1;
  int fd1 = -1;
  int fd2 = -1;
  int i = 0;

  for (i = 0; i < 300; i++) {
    long_file[i] = (i % 2 == 0) ? '1' : '2';
  }
  long_file[i] = '\0';

  printf("\n-------Testing tfs_mkfs with a bad file size--------\n");
  testMakeFsBadSize();

  printf("\n-------Testing tfs_mkfs with a good file size--------\n");
  testMakeFsGood();

  printf("\n-------Testing tfs_mount with a good FS--------\n");
  testMount();

  printf("\n-------Testing tfs_openFile with a new file--------\n");
  fd1 = testOpenFile1();

  printf("\n-------Testing tfs_writeFile and tfs_readByte with small file--------\n");
  testReadWriteFile1(fd1, short_file);

  printf("\n-------Testing tfs_closeFile--------\n");
  testCloseFile(fd1);

  fd1 = tfs_openFile("new.txt");

  printf("\n-------Testing tfs_deleteFile--------\n");
  testDeleteFile(fd1);

  fd2 = tfs_openFile("n2.txt");

  printf("\n-------Testing tfs_writeFile with contents larger than 256 bytes--------\n");
  testReadWriteFile2(fd2, long_file);

  tfs_closeFile(fd2);

  printf("\n-------Testing Unmount and Remount--------\n");
  testUnmountAndRemount();

  printf("\n-------Testing files still on disk mount--------\n");
  testFilesExist();

  printf("\n-------Testing finished--------\n");
  if (demoStatus == 0) {
    printf("Demo succeeded with no problems\n");
  } else {
    printf("Demo failed on some parts, check output\n");
  }

  return 0;

}
