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
   superblock[SIZE_BYTE] = 0;
   status = writeBlock(disk_num, 0, superblock);
   free(superblock);
   return status;
}

freeBlock *addFreeBlock(freeBlock *tail, int block_num) {
   freeBlock *temp_block = malloc(sizeof(freeBlock));
   temp_block->next = NULL;
   temp_block->block_num = block_num;
   tail->next = temp_block;
   tail = tail->next;
   return tail;
}

int popFreeBlock() {
   int block_num = 0;
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
   char block[BLOCKSIZE];
   memset(block, 0, BLOCKSIZE);
   freeBlock *tail;
   head = malloc(sizeof(freeBlock));
   head->block_num = 0;
   head->next = NULL;
   tail = head;

   //start at 1 because of superblock
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
   return status;
}

int getFileSize(char *inode_buffer) {
   int size = 0;
   size += (int) inode_buffer[INODE_SIZE_BYTE_0];
   size += ((int) inode_buffer[INODE_SIZE_BYTE_1]) << 8;
   size += ((int) inode_buffer[INODE_SIZE_BYTE_2]) << 16;
   size += ((int) inode_buffer[INODE_SIZE_BYTE_3]) << 24;
   return size;
}

void getFileTimes(fileEntry *file, char *inode_buffer) {
   int i = 0;
   char *mod_time = malloc(TIMESTAMP_SIZE * sizeof(char));
   char *creat_time = malloc(TIMESTAMP_SIZE * sizeof(char));
   char *acc_time = malloc(TIMESTAMP_SIZE * sizeof(char));

   for (i = 0; i < TIMESTAMP_SIZE; i++) {
      creat_time[i] = inode_buffer[CREATE_TIME_BYTE + i];
   }
   creat_time[i] = '\0';

   for (i = 0; i < TIMESTAMP_SIZE; i++) {
      mod_time[i] = inode_buffer[MOD_TIME_BYTE + i];
   }
   mod_time[i] = '\0';

   for (i = 0; i < TIMESTAMP_SIZE; i++) {
      acc_time[i] = inode_buffer[ACCESS_TIME_BYTE + i];
   }

   acc_time[i] = '\0';

   file->modification_time = mod_time;
   file->creation_time = creat_time;
   file->access_time = acc_time;
}

int loadFiles() {
   int status = 0, i;
   char super_block_buf[BLOCKSIZE];
   char inode_buffer[BLOCKSIZE];
   char filename[9];
   fileEntry *file;

   readBlock(disk_num, 0, super_block_buf);
   total_files = (int) super_block_buf[SIZE_BYTE];
   for (i = 0; i < total_files; i++) {
      int index = 0;
      int inode_index = INODE_BYTE_START + i;
      file = malloc(sizeof(fileEntry));
      file->fd = i;
      file->inode_block_num = (int) super_block_buf[inode_index];
      file->num_copies = 0;

      status |= readBlock(disk_num, super_block_buf[inode_index], inode_buffer);
      while (inode_buffer[NAME_BYTE + index] != '\0' && index <= NAME_BYTE) {
         filename[index] = inode_buffer[NAME_BYTE + index];
         ++index;
      }
      filename[index] = '\0';
      file->name = strdup(filename);
      file->permissions = inode_buffer[PERMISSION_BYTE];
      file->file_size = getFileSize(inode_buffer);
      getFileTimes(file, inode_buffer);
      file_table[i] = file;
   }
   return status;
}

int getNumFreeBlocks() {
   int i = 0;
   freeBlock *temp = head;
   while (temp != NULL) {
      if (temp->block_num != 0) {
         ++i;
         temp = temp->next;
      }
   }
   return i;
}

freeBlock *getTail() {
   freeBlock *temp = head;
   while (temp->next != NULL) {
      temp = temp->next;
   }
   return temp;
}

int loadAllData() {
   int status = 0, cur_block = 0;
   freeBlock *tail;
   char buf[BLOCKSIZE];
   if (head != NULL) {
      return status;
   }
   head = checkedCalloc(sizeof(freeBlock));
   tail = head;
   head->block_num = 0;
   head->next = NULL;

   do {
      status |= readBlock(disk_num, cur_block, buf);
      cur_block = (int) buf[ADDR_BYTE];
      if (cur_block != 0) {
         tail = addFreeBlock(tail, cur_block);
      }
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
      status |= writeSuperblock();
      status |= setupFreeBlocks();
   }

   return status;
}

int tfs_mount(char *filename) {
   int status = 0;
   char block[BLOCKSIZE];
   if ((disk_num = openDisk(filename, 0)) < 0) {
      return DISK_ERROR;
   }
   status |= readBlock(disk_num, 0, block);
   if (block[MAGIC_BYTE] != MAGIC_NUMBER) {
      status |= DISK_ERROR;
   }
   num_blocks = fileSize(disk_num) / BLOCKSIZE;
   file_table = malloc(DEFAULT_BLOCK_AMOUNT * sizeof(fileEntry *));
   open_file_table = malloc(DEFAULT_BLOCK_AMOUNT * sizeof(openFile *));
   open_files = 0;
   total_files = 0;
   loadAllData();

   return status;
}

void updateINode(fileEntry *file) {
   int i, name_length = strlen(file->name);
   char buf[BLOCKSIZE];
   readBlock(disk_num, file->inode_block_num, buf);
   for (i = 0; i < NAME_BYTE; i++) {
      if (i > name_length) {
         buf[NAME_BYTE + i] = '\0';
      } else {
         buf[NAME_BYTE + i] = file->name[i];
      }
   }

   for (i = 0; i < TIMESTAMP_SIZE; i++) {
      buf[MOD_TIME_BYTE + i] = file->modification_time[i];
      buf[CREATE_TIME_BYTE + i] = file->creation_time[i];
      buf[ACCESS_TIME_BYTE + i] = file->access_time[i];
   }

   buf[PERMISSION_BYTE] = file->permissions;

   writeBlock(disk_num, file->inode_block_num, buf);
}

int saveAllData() {
   int i, status = 0, cur_block = 0, next_block = 0, valid_files = 0;//free_block_num = 0, write_block_num = 0;
   char buf[BLOCKSIZE];
   freeBlock *temp = head;
   char *empty_block = checkedCalloc(BLOCKSIZE);
   freeBlock *cur;
   readBlock(disk_num, 0, buf);

   for (i = 0; i < total_files; i++) {
      if (file_table[i]->inode_block_num != 0) {
         buf[SIZE_BYTE + valid_files] = file_table[i]->inode_block_num;
         updateINode(file_table[i]);
         ++valid_files;
      }
   }
   buf[SIZE_BYTE] = valid_files;
   status |= writeBlock(disk_num, 0, buf);

   //no free blocks to save
   if (temp->block_num == 0) {
      free(empty_block);
      return status;
   }

   readBlock(disk_num, 0, buf);
   cur_block = temp->block_num;
   buf[ADDR_BYTE] = cur_block;
   status |= writeBlock(disk_num, 0, buf);
   cur = temp;
   temp = temp->next;
   free(cur);
   while (temp != NULL) {
      cur = temp;
      next_block = temp->block_num;
      empty_block[TYPE_BYTE] = FREE_BLOCK;
      empty_block[MAGIC_BYTE] = MAGIC_NUMBER;
      empty_block[ADDR_BYTE] = next_block;
      status |= writeBlock(disk_num, cur_block, empty_block);
      cur_block = next_block;
      temp = temp->next;
      free(cur);
   }
   next_block = 0;
   empty_block[ADDR_BYTE] = next_block;
   status |= writeBlock(disk_num, cur_block, empty_block);
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
   int i;
   int new_file = 1;
   openFile *open_file = malloc(sizeof(openFile));
   fileEntry *file;

   for (i = 0; i < total_files; i++) {
      if (strcmp(file_table[i]->name, name) == 0) {
         printf("copy\n");
         file = file_table[i];
         file_table[i]->num_copies++;
         new_file = 0;
         break;
      }
   }

   if (new_file) {
      file = malloc(sizeof(fileEntry));
      file->name = strdup(name);
      file->creation_time = strdup(getCurrentTime());
      file->modification_time = strdup(file->creation_time);
      file->access_time = strdup(file->creation_time);
      file->fd = total_files;
      file->num_copies = 1;
      file->inode_block_num = 0;
      file->permissions = READ_WRITE;
      file->file_size = 0;
      file_table[total_files++] = file;
   }
   open_file->fd = open_files;
   open_file->file_index = file->fd;
   open_file->first_block = 0;
   open_file->cur_block = 0;
   open_file->cur_position = 0;
   if (file_table[open_file->file_index]->inode_block_num != 0) {
      char buf[BLOCKSIZE];
      readBlock(disk_num, file_table[open_file->file_index]->inode_block_num, buf);
      open_file->first_block = (int) buf[ADDR_BYTE];
      open_file->cur_block = open_file->first_block;
   }

   open_file_table[open_files++] = open_file;
   return open_file->fd;
}

void shiftOpenFileTable(int index) {
   //int i;
   free(open_file_table[index]);
   //for (i = index; i < open_files - 1; i++) {
   //   open_file_table[i] = open_file_table[i + 1];
   //}
}

void shiftFileTable(int index) {
   int i;
   free(file_table[index]);
   for (i = index; i < total_files - 1; i++) {
      file_table[i] = file_table[i + 1];
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

// Returns the file's fd using the file name
int findFileEntry(char* filename) {
	int i;
	for (i = 0; i < total_files; ++i) {
		if (strcmp(file_table[i]->name, filename) == 0) {
			return i;
		}
	}
	return -4;
}

int getOpenFile(fileDescriptor fd) {
   int i;
   for (i = 0; i < open_files; i++) {
      if (open_file_table[i]->fd == fd) {
         return i;
      }
   }
   return -1;
}

int getFileEntry(fileDescriptor fd) {
   int i;
   for (i = 0; i < total_files; i++) {
      if (file_table[i]->fd == fd) {
         return i;
      }
   }
   return -1;
}

char *initINode(openFile *file, fileEntry *file_entry, int size) {
   int i, name_size = strlen(file_entry->name);
   char *block = checkedCalloc(BLOCKSIZE);
   block[TYPE_BYTE] = INODE;
   block[MAGIC_BYTE] = MAGIC_NUMBER;
   block[PERMISSION_BYTE] = READ_WRITE;
   for (i = 0; i < NAME_BYTE; i++) {
      if (i >= name_size) {
         block[NAME_BYTE + i] = '\0';
      } else {
         block[NAME_BYTE + i] = file_entry->name[i];
      }
   }
   block[INODE_SIZE_BYTE_0] = size & 0xFF;
   block[INODE_SIZE_BYTE_1] = (size >> 8) & 0xFF;
   block[INODE_SIZE_BYTE_2] = (size >> 16) & 0xFF;
   block[INODE_SIZE_BYTE_3] = (size >> 24) & 0xFF;
   return block;
}

void printBlockChain(int start_block) {
   char buf[BLOCKSIZE];
   readBlock(disk_num, start_block, buf);
   int next_block = (int) buf[ADDR_BYTE];
   printf("start %d\n", start_block);
   while (next_block != 0) {
      printf("next %d\n", next_block);
      readBlock(disk_num, next_block, buf);
      next_block = (int) buf[ADDR_BYTE];
   }
}

int tfs_writeFile(fileDescriptor fd, char *buffer, int size) {
   int i, status = 0;
   int num_free_blocks = getNumFreeBlocks();
   int num_blocks_needed = size / DATA_BLOCK_SIZE + 2; //1 inode block + 1 data block;
   int open_file_index = getOpenFile(fd);
   int buffer_index = 0;
   openFile *file = open_file_table[open_file_index];
   fileEntry *file_entry = file_table[file->file_index];
   char *inode_block, *temp_data_block = checkedCalloc(BLOCKSIZE);
   int next_block;
   if (file == NULL) {
      printf("File not found\n");
      return FILE_NOT_FOUND;
   } else if (num_blocks_needed > num_free_blocks) {
      printf("Not enough blocks to write file\n");
      return WRITE_FAIL;
   } else if (size == 0) {
      printf("Must pass in buffer and size to write\n");
      return WRITE_FAIL;
   } else if (file_entry->permissions == READ_ONLY) {
      printf("Cannot write over read only file\n");
      return BAD_PERMISSIONS;
   }
   int inode_block_num = popFreeBlock();
   file_entry->file_size = size;
   file_entry->inode_block_num = inode_block_num;
   inode_block = initINode(file, file_entry, size);
   next_block = popFreeBlock();
   file->first_block = next_block;
   file->cur_block = next_block;
   for (i = 0; i < num_blocks_needed - 1; i++) {
      int block_index = DATA_START_BYTE;
      int cur_block = next_block;
      if (inode_block[ADDR_BYTE] == 0) {
         inode_block[ADDR_BYTE] = next_block;
         status |= writeBlock(disk_num, inode_block_num, inode_block);
      }
      temp_data_block[TYPE_BYTE] = FILE_EXTENT;
      while (block_index != BLOCKSIZE && buffer_index != size) {
         temp_data_block[block_index++] = buffer[buffer_index++];
      }
      if (buffer_index == size) {
         temp_data_block[ADDR_BYTE] = 0;
      } else {
         next_block = popFreeBlock();
         temp_data_block[ADDR_BYTE] = next_block;
      }
      status |= writeBlock(disk_num, cur_block, temp_data_block);
   }

   free(inode_block);
   free(temp_data_block);
   file->cur_position = 0;
   return status;
}

int tfs_deleteFile(fileDescriptor fd) {
   int status = 0;
   int open_file_index = getOpenFile(fd);
   int tail_value = getTail()->block_num;
   int free_list_empty = 0;
   int cur_block;
   freeBlock *tail = getTail();
   if (open_file_index == -1) {
      return FILE_NOT_FOUND;
   }
   openFile *file = open_file_table[open_file_index];
   fileEntry *file_entry = file_table[file->file_index];
   int file_table_index = file_entry->fd;
   if (tail_value <= 0) {
      free_list_empty = 1;
   }
   if (file_entry->permissions == READ_ONLY) {
      printf("Can't delete read only file\n");
      return(BAD_PERMISSIONS);
   }
   cur_block = file_entry->inode_block_num;
   while (cur_block != 0) {
      int new_free_block = cur_block;
      char *read_block = checkedCalloc(BLOCKSIZE);
      char *write_block = checkedCalloc(BLOCKSIZE);
      status |= readBlock(disk_num, cur_block, read_block);
      cur_block = (int) read_block[ADDR_BYTE];
      write_block[TYPE_BYTE] = FREE_BLOCK;
      write_block[MAGIC_BYTE] = MAGIC_NUMBER;
      write_block[ADDR_BYTE] = cur_block;
      status |= writeBlock(disk_num, new_free_block, write_block);
      tail = addFreeBlock(tail, new_free_block);
      free(read_block);
      free(write_block);
   }

   shiftFileTable(file_table_index);
   shiftOpenFileTable(open_file_index);
   --total_files;
   --open_files;
   if (free_list_empty) {
      popFreeBlock();
   }
   return status;
}

int tfs_readByte(fileDescriptor fd, char *buffer) {
   int status = 0;
   char cur_block_data[BLOCKSIZE];
   int open_file_index = getOpenFile(fd);
   // Obtaining the openFile we are reading from
   openFile *file = open_file_table[open_file_index];
   int file_table_index = getFileEntry(file->file_index);
   if (file == NULL) {
      printf("Cannot find file in open file table\n");
   	return FILE_NOT_FOUND;
   }

   if (file->cur_block == 0) {
      printf("Can't read from empty file\n");
      return READ_FILE_ERROR;
   }

   // Check to see if cur_position is already at the end of the file
   if(file->cur_position >= file_table[file_table_index]->file_size) {
      printf("Already reached the end of the file\n");
   	return WRITE_FAIL;
   }

   status |= readBlock(disk_num, file->cur_block, cur_block_data);

	if(status < 0) {
		printf("Readblock failed.\n");
		return status;
	}

	// Save the read byte to index 0 of the buffer
	int data_position = DATA_START_BYTE + file->cur_position % DATA_BLOCK_SIZE;
	buffer[0] = cur_block_data[data_position];

	// Increments pointer position
   ++(file->cur_position);


   return status;
}


int tfs_seek(fileDescriptor fd, int offset) {
   int status = 0;

   char cur_block_data[BLOCKSIZE];


   int open_file_index = getOpenFile(fd);
   openFile *file = open_file_table[open_file_index];
   int file_table_index = file->file_index;

   if (file == NULL) {
      return FILE_NOT_FOUND;
   }

   if (offset > file_table[file_table_index]->file_size) { // Offset goes past the actual file size
   	return SEEK_FAIL;
   }

   file->cur_position = offset;

	int num_blocks_traversed = offset / DATA_BLOCK_SIZE;
	file->cur_block = file->first_block;

	int i;
	for(i = 0; i < num_blocks_traversed; ++i)
	{
		status |= readBlock(disk_num, file->cur_block, cur_block_data); // Getting contents of block to get next addr

		if(status < 0) {
			return status;
		}

		file->cur_block = (int) cur_block_data[ADDR_BYTE];
	}

	//Change access time
	file_table[file_table_index]->access_time = getCurrentTime();

	return status;
}

// Renames a given file (by filename) to a new name
int tfs_rename(char* old_name, char* new_name) {
	int status = 0;

	int file_table_index = findFileEntry(old_name);

	// Check to see if file exists
	if(file_table_index < 0) {
		printf("File not found.\n");
		return FILE_NOT_FOUND;
	}

	// Renames file + sets modification
	file_table[file_table_index]->name = realloc(file_table[file_table_index]->name, strlen(new_name));
	file_table[file_table_index]->name = new_name;

	if(file_table[file_table_index]->name == NULL)
	{
		return CALLOC_ERROR;
	}

	file_table[file_table_index]->modification_time = getCurrentTime();
	file_table[file_table_index]->access_time = getCurrentTime();

	return status;
}

// Traverses file table and prints out file names
int tfs_readdir() {
	int status = 0;
	int i;

	// Traverse through every file entry
	for(i = 0; i < total_files; ++i) {
		printf("%s ", file_table[i]->name);
	}
   printf("\n");

	printf("\n");
	return status;
}

// Makes a file read only
int tfs_makeRO(char *filename) {
	int status = 0;
	int file_table_index = findFileEntry(filename);

	// Check to see if file exists
	if(file_table_index < 0) {
		printf("File not found.\n");
		return FILE_NOT_FOUND;
	}

	// Changes permission byto on a file + sets modification
	file_table[file_table_index]->permissions = READ_ONLY;
	file_table[file_table_index]->access_time = getCurrentTime();
	file_table[file_table_index]->modification_time = getCurrentTime();
   char num[26];
   strcpy(num, getCurrentTime());
   num[26] = 0;
   printf("%s\n", num);
	return status;
}

// Gives a file read and write permissions
int tfs_makeRW(char *filename) {
	int status = 0;
	int file_table_index = findFileEntry(filename);

	// Check to see if file exists
	if(file_table_index < 0) {
		printf("File not found.\n");
		return FILE_NOT_FOUND;
	}

	// Changes permission byto on a file + sets modification
	file_table[file_table_index]->permissions = READ_WRITE;
	file_table[file_table_index]->access_time = getCurrentTime();
	file_table[file_table_index]->modification_time = getCurrentTime();

	return status;
}

// Writes a byte to an exact position inside the file to location 'offset'(absolute)
int tfs_writeByte(fileDescriptor fd, int offset, unsigned char data) {
   int status = 0;
   char cur_block_data[BLOCKSIZE];
   int open_file_index = getOpenFile(fd);

   // Obtaining the openFile we are reading from
   openFile *file = open_file_table[open_file_index];
   int file_table_index = getFileEntry(file->file_index);
   if (file == NULL) {
      printf("Cannot find file in open file table\n");
   	return FILE_NOT_FOUND;
   } else if (file->cur_block == 0) {
      printf("Can't read from empty file\n");
      return READ_FILE_ERROR;
   } else if (file_table[file_table_index]->permissions == READ_ONLY) {
      printf("Can't write over read only file\n");
      return BAD_PERMISSIONS;
   }

   // Change the current position in the file to the new offset
   file->cur_position = offset;

   // Check to see if cur_position is already at the end of the file
   if(file->cur_position >= file_table[file_table_index]->file_size) {
      printf("Already reached the end of the file\n");
   	return WRITE_FAIL;
   }

   status |= readBlock(disk_num, file->cur_block, cur_block_data);
	if(status < 0) {
		//readBlock failed
		printf("Readblock failed.\n");
		return status;
	}

	// Calculate the byte location in the block and update the byte
	int block_location = file->cur_position % DATA_BLOCK_SIZE;
	cur_block_data[block_location] = data;

	file_table[file_table_index]->access_time = getCurrentTime();
	file_table[file_table_index]->modification_time = getCurrentTime();

	// Increments pointer position (from offset)
   ++(file->cur_position);

   return status;
}

int tfs_readFileInfo(int fd) {
	int status = 0;
	//int file_table_index = getFileEntry(fd);
   openFile *file = open_file_table[fd];
   int file_table_index = file->file_index;
	char* filename = file_table[file_table_index]->name;

	if(file_table_index < 0) {
		printf("File not found. \n");
		return FILE_NOT_FOUND;
	}

	printf("File name: %s\n", filename);
	printf("Creation time: %s\n", file_table[file_table_index]->creation_time);
	printf("Last accessed: %s\n", file_table[file_table_index]->access_time);
	printf("Last modified: %s\n", file_table[file_table_index]->modification_time);

	// Update access time
	file_table[file_table_index]->access_time = getCurrentTime();

	return status;
}
