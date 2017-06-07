#include "libTinyFS.h"
#include "libDisk.h"
#include "TinyFS_errno.h"

int num_blocks;
int disk_num = -1;
int open_files;
int total_files;
fileDescriptor next_fd = 0;
fileEntry **file_table;
openFile **open_file_table;
freeBlock* head;


void *checkedCalloc(int size) {
   void *data = calloc(1, size);
   if (data == NULL) {
      fprintf(stderr, "Failed to allocate blocks to disk\n");
      exit(CALLOC_ERROR);
   }
   return data;
}

int writeSuperblock() {
   char *superblock = checkedCalloc(BLOCKSIZE);
   int status;
   superblock[TYPE_BYTE] = SUPERBLOCK;
   superblock[MAGIC_BYTE] = MAGIC_NUMBER;
   superblock[ADDR_BYTE] = 0;
   superblock[SIZE_BYTE] = num_blocks;
   status = writeBlock(disk_num, 0, superblock);
   return status;
}

freeBlock *addFreeBlock(freeBlock *tail, int block_num) {
   freeBlock *temp_block = checkedCalloc(sizeof(freeBlock));
   temp_block->next = NULL;
   temp_block->block_num = block_num;
   tail->next = temp_block;
   tail = tail->next;
   return tail;
}

int popFreeBlock() {
   int block_num = -1;
   freeBlock *temp = head;
   block_num = head->block_num;
   if (head->next != NULL) {
      head = head->next;
      free(temp);
   } else {
      head->block_num = 0;
   }
   return block_num;
}

int setupFreeBlocks() {
   int i;
   int status = 0;
   char *block = checkedCalloc(sizeof(BLOCKSIZE));
   freeBlock *tail;
   head = checkedCalloc(sizeof(freeBlock));
   head->block_num = 0;
   head->next = NULL;
   tail = head;

   for (i = 1; i < num_blocks; i++) {
      int free_block_num = i + 1;
      if (i + 1 == num_blocks) {
         free_block_num = 0;
      }
      block[TYPE_BYTE] = FREE_BLOCK;
      block[MAGIC_BYTE] = MAGIC_NUMBER;
      block[ADDR_BYTE] = free_block_num;
      status |= writeBlock(disk_num, i, block);
      tail = addFreeBlock(tail, i);
   }
   popFreeBlock();
   free(block);
   return status;
}

int loadFiles() {
   int status = 0, i;
   char super_block_buf[BLOCKSIZE];
   char inode_buffer[BLOCKSIZE];
   char filename[9];
   fileEntry *file;

   readBlock(disk_num, 0, super_block_buf);
   total_files = super_block_buf[SIZE_BYTE];
   for (i = 0; i < total_files; i++) {
      int index = 0;
      int inode_index = SIZE_BYTE + i + 1;
      file = checkedCalloc(sizeof(fileEntry));
      file->fd = i;
      file->inode_block_num = super_block_buf[inode_index];
      file->num_copies = 0;

      status |= readBlock(disk_num, super_block_buf[inode_index], inode_buffer);
      while (inode_buffer[NAME_BYTE + index] != '\0' && index < NAME_BYTE) {
         filename[index] = inode_buffer[NAME_BYTE + index];
         ++index;
      }
      filename[index] = '\0';
      file->name = filename;
      file->permissions = inode_buffer[PERMISSION_BYTE];
      getFileTimes(file, inode_buffer);
      file_table[i] = file;
   }
   return status;
}

freeBlock *getTail() {
   freeBlock *temp = head;
   while (temp->next != NULL) {
      temp = temp->next;
   }
   return temp;
}

int loadAllData() {
   int status = 0, cur_block = 0, prev_block = 0;
   char buf[BLOCKSIZE];
   freeBlock *temp;
   if (head != NULL) {
      return status;
   }
   head = checkedCalloc(sizeof(freeBlock));
   temp = head;

   head->block_num = 0;
   head->next = NULL;

   do {
      prev_block = cur_block;
      status |= readBlock(disk_num, cur_block, buf);
      cur_block = (int) buf[ADDR_BYTE];
      addFreeBlock(getTail(), cur_block);
   } while (cur_block != 0);

   popFreeBlock();

   status |= loadFiles();

   return status;
}

int tfs_mkfs(char *filename, int nBytes) {
   int disk_fd;
   int status = 0;
   if ((disk_fd = openDisk(filename, nBytes)) < 0) {
      fprintf(stderr, "Failed to open disk\n");
      return DISK_ERROR;
   }
   disk_num = disk_fd;
   num_blocks = nBytes / BLOCKSIZE;
   if (nBytes > 0) {
      status |= writeSuperBlock();
      status |= setupFreeBlocks();
   }

   return status;
}

int tfs_mount(char *filename) {
   int status = 0;
   char block[256];
   if ((disk_num = openDisk(filename, 0)) < 0) {
      return DISK_ERROR;
   }
   file_table = checkedCalloc(num_blocks * sizeof(fileEntry));
   open_file_table = checkedCalloc(num_blocks * sizeof(openFile));
   open_files = 0;
   total_files = 0;
   loadAllData();
   status |= readBlock(disk_num, 0, block);
   if (block[MAGIC_BYTE] != MAGIC_NUMBER) {
      status |= DISK_ERROR;
   }
   return status;
}

int saveAllData() {
   int i, status = 0, free_block = 0, write_block = 0;;
   char buf[BLOCKSIZE];
   freeBlock *temp = head;
   char *empty_block = checkedCalloc(BLOCKSIZE);

   readBlock(disk_num, 0, buf);
   buf[SIZE_BYTE] = total_files;
   for (i = 1; i <= total_files; i++) {
      buf[SIZE_BYTE + i] = file_table[i - 1]->inode_block_num;
   }
   status |= writeBlock(disk_num, 0, buf);

   readBlock(disk_num, 0, buf);
   free_block = temp->block_num;
   empty_block[ADDR_BYTE] = temp->block_num;
   status |= writeBlock(disk_num, write_block, empty_block);
   temp = temp->next;
   write_block = free_block;

   empty_block[TYPE_BYTE] = FREE_BLOCK;
   empty_block[MAGIC_BYTE] = MAGIC_NUMBER;
   while (temp != NULL) {
      free_block = temp->block_num;
      empty_block[ADDR_BYTE] = free_block;
      status |= writeBlock(disk_num, write_block, empty_block);
      temp = temp->next;
      write_block = free_block;
   }
   free_block = 0;
   empty_block[ADDR_BYTE] = free_block;
   status |= writeBlock(disk_num, write_block, empty_block);
   free(empty_block);

   return status;
}

int tfs_unmount(void) {
   saveAllData();
   free(file_table);
   free(open_file_table);   
   return 0;
}

char *getCurrentTime() {
   time_t raw_time;
   struct tm *time_info;
   char* time_string;
   time(&raw_time);
   time_info = localtime(&raw_time);
   time_string = asctime(time_info);
   return time_string;
}

fileDescriptor tfs_openFile(char *name) {
   int i = -1, found = 0;
   char block[BLOCKSIZE];
   openFile *open_file = checkedCalloc(sizeof(openFile));
   fileEntry *file;

   for (i = 0; i < total_files; i++) {
      if (strcmp(file_table[i]->name, name) == 0) {
         file = file_table[i];
         file_table[i]->num_copies++;
         break;
      }
   }
   if (i == -1) {
      file = checkedCalloc(sizeof(fileEntry));
      file_table[total_files++] = file;
      file->name = name;
      file->creation_time = getCurrentTime();
      file->modification_time = file->creation_time;
      file->access_time = file->creation_time;
      file->fd = total_files;
      file->num_copies = 1;
      file->inode_block_num = -1;
      file->permissions = READ_WRITE;
   }
   open_file->fd = open_files;
   open_file->file_index = file->fd;
   open_file->first_block = -1;
   if (file_table[open_file->file_index]->inode_block_num != 0) {
      char buf[BLOCKSIZE];
      readBlock(disk_num, file_table[open_file->file_index]->inode_block_num, buf);
      open_file->first_block = (int) buf[ADDR_BYTE];
   }
   open_file->cur_position = 0;
   open_file_table[open_files++] = open_file;
   return open_file->fd;
}

void shiftOpenFileTable(int index) {
   int i;
   free(open_file_table[index]);
   for(i = index; i < open_files - 1; i++){
      open_file_table[i] = open_file_table[i+1];
   }
}

int tfs_closeFile(fileDescriptor fd) {
   int i, status = 0;
   for (i = 0; i < open_files; i++) {
      if (open_file_table[i]->fd == fd) {
         int file_table_index = open_file_table[i]->file_index;
         shiftOpenFileTable(i);
         file_table[file_table_index]->num_copies--;
         --open_files;
         break;
      }
   }
   return status;
}

int tfs_writeFile(fileDescriptor fd, char *buffer, int size) {
   int status = 0;
   return status;
}

int tfs_deleteFile(fileDescriptor fd) {
   int status = 0;
   return status;
}

int tfs_readByte(fileDescriptor fd, char *buffer) {
   int status = 0;
   return status;
}

int tfs_seek(fileDescriptor fd, int offset) {
   int status = 0;
   return status;
}
