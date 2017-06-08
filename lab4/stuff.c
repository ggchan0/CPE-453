Skip to content
Features Business Explore Marketplace Pricing
This repository
Search
Sign in or Sign up
 Watch 3  Star 1  Fork 1 luke-plewa/TinyFS
 Code  Issues 0  Pull requests 0  Projects 0 Insights
Branch: master Find file Copy pathTinyFS/libTinyFS.c
c1a53c8  on Mar 16, 2014
 Andrew Caddell Merge branch 'master' of https://github.com/luke-plewa/TinyFS
2 contributors @luke-plewa @synhyborex
RawBlameHistory
719 lines (643 sloc)  19.6 KB
#include "libTinyFS.h"

int tfs_mkfs(char *filename, int nBytes) {
  int disk_num;
  if((disk_num = openDisk(filename,nBytes)) < 0) {
    printf("Error opening disk.\n");
    return disk_num;
  }
  global_disk_num = disk_num;
  disk_size = nBytes / BLOCKSIZE;

  if (nBytes != 0) {
    writeSuper();
    writeFreeblocks();
  }

  return 0;
}

int writeSuper() {
  char* block = (char*)calloc(1, BLOCKSIZE);
  if(block == NULL){
    printf("Could not allocate blocks for disk.\n");
    return WRITE_FILE_FAIL;
  }
  //set starting bytes of block
  block[0] = SUPERBLOCK;
  block[1] = MAGIC;
  block[2] = 0; // first free block
  block[3] = 0;
  block[DISK_SIZE_BYTE] = disk_size & BIT_MASK;
  return writeBlock(global_disk_num, 0, block);
}

int writeFreeblocks() {
  int i;
  char* block = (char*)calloc(1, BLOCKSIZE);
  free_head = malloc(sizeof(freeBlock));
  free_head->address = 0;
  free_head->next = NULL;

  if (block == NULL) {
    printf("Could not allocate blocks for disk.\n");
    return WRITE_FILE_FAIL;
  }

  for (i = 1; i < disk_size; i++) {
    block[0] = FREE_BLOCK;
    block[1] = MAGIC;
    block[2] = i+1;
    if (i + 1 == disk_size) {
      block[2] = 0;
    }
    writeBlock(global_disk_num, i, block);
    addEmptyBlock(i);
  }
  // pop off the empty head
  getEmptyBlock();
  free(block);

  return 0;
}

int tfs_mount(char *filename) {
  global_disk_num = openDisk(filename, 0);
  if (global_disk_num < 0) {
    return OPEN_FILE_FAIL;
  }
  master_filetable = malloc(MAX_TABLE_ENTRIES*sizeof(file));
  open_filetable = malloc(MAX_TABLE_ENTRIES*sizeof(open_file));
  openFiles = 0;
  totalFiles = 0;
  // get disk size
  populateFreeBlocks();
  populateFiles();
  //to check correct format, just verify that the magic number is equal to MAGIC
  char block[1];
  readBlockByte(global_disk_num,0,1,block);
  if(block[0] != MAGIC){
    return OPEN_FILE_FAIL;
  }
  readBlockByte(global_disk_num, 0, DISK_SIZE_BYTE, block);
  disk_size = (int) *block;
  return 0;
}

int tfs_unmount(void) {
  free(master_filetable);
  free(open_filetable);
  saveFreeBlocks();
  saveFiles();
  return 0;
}

fileDescriptor tfs_openFile(char *name) {
  int i, found = 0;
  char buffer[BLOCKSIZE];
  char* filename;
  open_file* openFile = malloc(sizeof(open_file));

  for(i = 0; i < totalFiles && !found; i++) {
    if(!strcmp(name, master_filetable[i]->name)) {
      found = 1;
      master_filetable[i]->numInstances++;
      master_filetable[i]->access_time = getLocalTime();
      modifyInodeTimestamp(i, ACCESS);
      i--;
    }
  }
  if(!found) {
    file* newFile = malloc(sizeof(file));
    newFile->fd = totalFiles;
    master_filetable[totalFiles++] = newFile;
    newFile->inode_bNum = 0;
    newFile->numInstances = 1;
    newFile->name = name;
    newFile->permissions = READ_WRITE;
    newFile->creation_time = getLocalTime();
    newFile->mod_time = newFile->creation_time;
    newFile->access_time = newFile->creation_time;
  }

  openFile->fd = openFiles;
  openFile->master_index = master_filetable[i]->fd;
  openFile->start_bNum = 0;
  if (master_filetable[openFile->master_index]->inode_bNum != 0) {
    // fill in start bnum if it exists, so we can write
    char buff[BLOCKSIZE];
    readBlock(global_disk_num, master_filetable[openFile->master_index]->inode_bNum, buff);
    openFile->start_bNum = (int) buff[BLOCK_ADDR_BYTE];
  }
  openFile->position_pointer = 0;
  open_filetable[openFiles] = openFile;

  return open_filetable[openFiles++]->fd;
}

int tfs_closeFile(fileDescriptor FD) {
  int i;
  for(i = 0; i < openFiles; i++) {
    if(open_filetable[i]->fd == FD) {
      //update open file table
      moveOpenFileTable(i);
      //decrement number of instances in master table
      master_filetable[open_filetable[i]->master_index]->numInstances--;
      openFiles--;
      return 0;
    }
  }

  return FILE_NOT_FOUND; //if it gets here, the given FD wasn't in the table
}

int tfs_writeFile(fileDescriptor FD, char *buffer, int size) {
  int offset = size / BLOCKSIZE;
  int blocksNeeded = offset + 1;
  char my_buff[BLOCKSIZE];
  int index = 0;
  int buff_index = BYTE_OFFSET;
  int curr_block = 0;
  int next_block = 0;

  if(!hasWritePermission(FD)){
    printf("The file is read-only\n");
    return PERMISSIONS_FAIL;
  }
  if (blocksNeeded > freeBlocks()) {
    printf("Not enough size on disk to write the file\n");
    return WRITE_FILE_FAIL;
  }
  offset = 0;
  curr_block = writeInode(FD, size);
  if (curr_block == -1) {
    return WRITE_FILE_FAIL;
  }
  while (index < size) {
    if (needNewBlock(index, offset)) {
      if (index + 1 == size || size < BLOCKSIZE + offset) {
        next_block = 0;
      } else {
        next_block = getEmptyBlock();
      }
      prepareBuffer(my_buff, curr_block, FILE_EXTENT, next_block);
      writeBlock(global_disk_num, curr_block, my_buff);
      offset += BYTE_OFFSET;
      memset(my_buff, '\0', BLOCKSIZE);
      buff_index = BYTE_OFFSET;
      curr_block = next_block;
    }
    my_buff[buff_index] = buffer[index];
    index++;
  }
  modifyInodeTimestamp(open_filetable[FD]->master_index, CREATION);
  master_filetable[open_filetable[FD]->master_index]->mod_time = getLocalTime();
  modifyInodeTimestamp(open_filetable[FD]->master_index, MODIFICATION);
  return 0;
}

int tfs_deleteFile(fileDescriptor FD) {
  int i;
  for(i = 0; i < totalFiles; i++) {
    if(master_filetable[i]->fd == FD) {
      if(master_filetable[i]->numInstances == 0){
        if(!hasWritePermission(FD)){
          printf("Cannot delete read-only file\n");
          return PERMISSIONS_FAIL;
        }
        //update master file table
        moveMasterFileTable(i);
        totalFiles--;
      }
      else {
        printf("Can't delete file while there are open instances.\n");
        return OPEN_INSTANCES_FAIL;
      }
      return 0;
    }
  }

  return FILE_NOT_FOUND; //if it gets here, the given FD wasn't in the table
}

int tfs_readByte(fileDescriptor FD, char *buffer) {
  // TODO: error handling when at end of file
    //added in, maybe we want to add more error handling. check comments.
  // TODO: need handling for position and block transitions
    //just needed to readBlock() from current_block instead of start_bNum
  open_file* file = getOpenFile(FD);
  char buff[1];
  if (file == NULL || file->start_bNum == 0) {
    // nothing to read
    return READ_FILE_FAIL;
  }
  //could be replaced by readBlockByte()
  int error = readBlockByte(global_disk_num, file->current_block,
                            file->position_pointer, buff);
  if(error == READ_FILE_FAIL){
    return READ_FILE_FAIL;
  }
  if (file->position_pointer >= BLOCKSIZE) {
    //transition to next block
    file->current_block = buff[0];
    file->position_pointer = BYTE_OFFSET;
    error = readBlockByte(global_disk_num, file->current_block,
                          file->position_pointer, buff);
    if(error == READ_FILE_FAIL){
      return READ_FILE_FAIL;
    }
  }
  //printf("%c\n", buff[file->position_pointer]);
  file->position_pointer++;
  buffer[0] = buff[0];
  //readBlockByte(global_disk_num,file->current_block,file->position_pointer,buffer);
  master_filetable[open_filetable[FD]->master_index]->access_time = getLocalTime();
  modifyInodeTimestamp(open_filetable[FD]->master_index, ACCESS);
  return 0;
}

int tfs_writeByte(fileDescriptor FD, unsigned int data){
  open_file* file = getOpenFile(FD);
  char buff[1];
  buff[0] = data;
  char new_buff[BLOCKSIZE];
  char temp_buff[BLOCKSIZE];
  int curr_block = 0;
  int next_block = 0;
  if (file == NULL || file->start_bNum == 0) {
    // nothing to read
    return READ_FILE_FAIL;
  }

  file->position_pointer++;
  if (file->position_pointer >= BLOCKSIZE) {
    curr_block = getEmptyBlock();
    readBlock(global_disk_num, file->current_block, new_buff);
    new_buff[BLOCK_ADDR_BYTE] = curr_block;
    prepareBuffer(new_buff,file->current_block,FILE_EXTENT,next_block);
    writeBlock(global_disk_num,curr_block,new_buff);
    file->current_block = curr_block;
    file->position_pointer = BYTE_OFFSET;
  }
  int error = writeBlockByte(global_disk_num,file->current_block,file->position_pointer,buff);
  if(error < 0){
    //return the error code
    return error;
  }
  return 0;
}

int tfs_seek(fileDescriptor FD, int offset) {
  open_file* file = getOpenFile(FD);
  int position = offset;
  int curr_block = 0;
  char buff[1];

  if (file == NULL) {
    return FILE_NOT_FOUND;
  }
  curr_block = file->start_bNum;
  while (position > BLOCKSIZE - BYTE_OFFSET) {
    ///printf("%d %d\n",position, BLOCKSIZE - BYTE_OFFSET);
    //next 2 lines could be rewritten using readBlockByte()
    //needs testing first
    readBlockByte(global_disk_num, curr_block, 2, buff);
    curr_block = buff[0];
    position -= BLOCKSIZE - BYTE_OFFSET;
  }
  file->position_pointer = position+BYTE_OFFSET;
  file->current_block = curr_block;
  master_filetable[open_filetable[FD]->master_index]->access_time = getLocalTime();
  modifyInodeTimestamp(open_filetable[FD]->master_index, ACCESS);
  return 0;
}

void tfs_readdir(){
  int i;
  for(i = 0; i < totalFiles; i++){
    printf("- %s\n",master_filetable[i]->name);
  }
}

void tfs_makeRO(char *name){
  char buff[BLOCKSIZE];
  int i, index = -1;

  for(i = 0; i < totalFiles; i++){
    if(!strcmp(name,master_filetable[i]->name)){
      master_filetable[i]->permissions = READ_ONLY;
      readBlock(global_disk_num, master_filetable[i]->inode_bNum, buff);
      index = i;
    }
  }
  if(index >= 0) {
    buff[BYTE_OFFSET+NAME_OFFSET+4] = READ_ONLY;
    writeBlock(global_disk_num, master_filetable[index]->inode_bNum, buff);
    master_filetable[index]->mod_time = getLocalTime();
    modifyInodeTimestamp(index, MODIFICATION);
  }
  else {
    fprintf(stderr, "File not found. Unable to change file permissions.\n");
  }
}

void tfs_makeRW(char *name){
  char buff[BLOCKSIZE];
  int i, index = -1;

  for(i = 0; i < totalFiles; i++){
    if(!strcmp(name,master_filetable[i]->name)){
      master_filetable[i]->permissions = READ_WRITE;
      readBlock(global_disk_num, master_filetable[i]->inode_bNum, buff);
      index = i;
    }
  }
  if(index >= 0) {
    buff[BYTE_OFFSET+NAME_OFFSET+4] = READ_WRITE;
    writeBlock(global_disk_num, master_filetable[index]->inode_bNum, buff);
    master_filetable[index]->mod_time = getLocalTime();
    modifyInodeTimestamp(index, MODIFICATION);
  }
  else {
    fprintf(stderr, "File not found. Unable to change file permissions.\n");
  }
}

int hasWritePermission(fileDescriptor fd){
  int i;
  for(i = 0; i < totalFiles; i++){
    if(master_filetable[i]->fd == master_filetable[open_filetable[fd]->master_index]->fd){
      return master_filetable[i]->permissions;
    }
  }
  return FILE_NOT_FOUND;
}

int prepareBlock(int bNum, unsigned char blockType, int nextBlock) {
  unsigned char blockHeader[4];

  blockHeader[0] = blockType;
  blockHeader[1] = MAGIC;
  blockHeader[2] = nextBlock;
  blockHeader[3] = EMPTY;
  return writeBlock(1, bNum, blockHeader);
}

void prepareBuffer(char* buff, int block_num, unsigned char block_type, int next_block) {
  buff[0] = block_type;
  buff[1] = MAGIC;
  buff[2] = next_block;
  buff[3] = EMPTY;
}

int getEmptyBlock() {
  int bNum = -1;
  freeBlock* temp = free_head;
  bNum = free_head->address;
  if (free_head->next != NULL) {
    free_head = free_head->next;
    free(temp);
  } else {
    free_head->address = 0;
  }
  return bNum;
}

freeBlock* getTailBlock() {
  freeBlock* current = free_head;
  while (current->next != NULL) {
    current = current->next;
  }
  return current;
}

void addEmptyBlock(int block_num) {
  // TODO maybe use push instead
  freeBlock* new = malloc(sizeof(freeBlock));
  new->next = NULL;
  new->address = block_num;
  freeBlock* tail = getTailBlock();
  tail->next = new;
}

int freeBlocks() {
  int count = 1;
  freeBlock* current = free_head;
  if (current == NULL || current->address == 0) {
    return 0;
  }
  while (current->next != NULL) {
    current = current->next;
    count++;
  }
  return count;
}

void moveOpenFileTable(int start) {
  free(open_filetable[start]);
  int i;
  for(i = start; i < openFiles-1; i++){
    //open_filetable[i] = open_filetable[i+1];
    memcpy(open_filetable[i],open_filetable[i+1],sizeof(open_file));
  }
}

void moveMasterFileTable(int start) {
  char byte[1];
  //read third byte to get block number of first block
  readBlockByte(global_disk_num,master_filetable[start]->inode_bNum,2,byte);
  //0 means no more blocks follow that one
  while(*byte != 0){
    //write first byte as free block
    char freeNum[1] = {FREE_BLOCK};
    writeBlockByte(global_disk_num,*byte,0,freeNum);
    addEmptyBlock(*byte); //add this block to the free block list
    *byte = readBlockByte(global_disk_num,*byte,2,byte); //get next block number
  }

  free(master_filetable[start]);
  int i;
  for(i = start; i < totalFiles-1; i++){
    memcpy(master_filetable[i],master_filetable[i+1],sizeof(file));
  }
}

int needNewBlock(int index, int offset) {
  return (index + offset * BYTE_OFFSET) % BLOCKSIZE == 0;
}

int writeInode(fileDescriptor fd, int size) {
  open_file* file = getOpenFile(fd);
  int block;
  char buff[BLOCKSIZE];
  int next_block = -1;

  if (file == NULL) {
    return FILE_NOT_FOUND;
  } else {
    block = getEmptyBlock();
    prepareBuffer(buff, block, INODE, 0);
    master_filetable[file->master_index]->inode_bNum = block;
    prepareInodeBuffer(buff, master_filetable[file->master_index]->name, size);
    if (size > 0) {
      next_block = getEmptyBlock();
      file->start_bNum = next_block;
      file->current_block = next_block;
      prepareBuffer(buff, block, INODE, next_block);
      writeBlock(global_disk_num, block, buff);
    } else {
      writeBlock(global_disk_num, block, buff);
    }
  }
  file->position_pointer = 0;
  return next_block;
}

void prepareInodeBuffer(char* buff, char* name, int size) {
  int index = 0;
  while (name[index] != '\0' && index < NAME_OFFSET) {
    buff[NAME_OFFSET+index] = name[index];
    index++;
  }
  buff[NAME_OFFSET+index] = '\0';
  buff[BYTE_OFFSET+NAME_OFFSET+0] = (size >> 24) & BIT_MASK;
  buff[BYTE_OFFSET+NAME_OFFSET+1] = (size >> 16) & BIT_MASK;
  buff[BYTE_OFFSET+NAME_OFFSET+2] = (size >> 8) & BIT_MASK;
  buff[BYTE_OFFSET+NAME_OFFSET+3] = size & BIT_MASK;
  buff[BYTE_OFFSET+NAME_OFFSET+4] = READ_WRITE;
}

void modifyInodeTimestamp(fileDescriptor fd, int permission_to_mod) {
  if(master_filetable[fd]->inode_bNum > 0) {
    char buff[BLOCKSIZE];
    char* timestamp = getLocalTime();
    int i, offset;

    readBlock(global_disk_num, master_filetable[fd]->inode_bNum, buff);

    switch(permission_to_mod) {
    case(CREATION):
      offset = CREATION_OFFSET;
      break;

    case(MODIFICATION):
      offset = MODIFICATION_OFFSET;
      break;

    case(ACCESS):
      offset = ACCESS_OFFSET;
      break;

    default:
      offset = -1;
      break;
    }

    for(i = 0; i > -1 && i < TIMESTAMP_SIZE; i++) {
      buff[offset+i] = timestamp[i];
    }
    writeBlock(global_disk_num, master_filetable[fd]->inode_bNum, buff);
  }
}

open_file* getOpenFile(fileDescriptor fd) {
  int index = 0;
  while (index < openFiles) {
    if (fd == open_filetable[index]->fd) {
      return open_filetable[index];
    }
    index++;
  }
  return NULL;
}

void tfs_rename(fileDescriptor fd, char* new_name) {
  char buff[BLOCKSIZE];
  int index = 0;

  readBlock(global_disk_num, master_filetable[fd]->inode_bNum, buff);
  while (new_name[index] != '\0' && index < NAME_OFFSET) {
    buff[NAME_OFFSET+index] = new_name[index];
    index++;
  }
  buff[NAME_OFFSET+index] = '\0';
  writeBlock(global_disk_num, master_filetable[fd]->inode_bNum, buff);
  master_filetable[fd]->name = new_name;
  master_filetable[fd]->mod_time = getLocalTime();
  modifyInodeTimestamp(fd, MODIFICATION);
}

char* tfs_readFileInfo(fileDescriptor fd) {
 /* puts("a");
  char* timestamp = "Creation Time: ";
  puts("1");
  strcat(timestamp, master_filetable[fd]->creation_time);
  puts("b");
  strcat(timestamp, "Last Modification: ");
  strcat(timestamp, master_filetable[fd]->mod_time);
  puts("c");
  strcat(timestamp, "Last Access: ");
  strcat(timestamp, master_filetable[fd]->access_time);
  puts("d");*/

  return master_filetable[fd]->mod_time;
}

char* getLocalTime() {
  time_t temp;
  struct tm* timeinfo;
  char* timestamp = malloc(TIMESTAMP_SIZE);

  time(&temp);
  timeinfo = localtime(&temp);
  timestamp = asctime(timeinfo);
  return timestamp;
}

void saveFreeBlocks() {
  freeBlock* temp = free_head;
  int address = 0;
  int write_block = 0;
  char block[BLOCKSIZE];
  char *empty_block = (char*) calloc(BLOCKSIZE, sizeof(char));

  // rewrite super block
  readBlock(global_disk_num, 0, block);
  address = temp->address;
  block[BLOCK_ADDR_BYTE] = address & BIT_MASK;
  writeBlock(global_disk_num, write_block, block);
  temp = temp->next;
  write_block = address;

  empty_block[0] = FREE_BLOCK;
  empty_block[1] = MAGIC;
  while (temp != NULL) {
    address = temp->address;
    empty_block[BLOCK_ADDR_BYTE] = address & BIT_MASK;
    writeBlock(global_disk_num, write_block, empty_block);
    temp = temp->next;
    write_block = address;
  }
  address = 0;
  empty_block[BLOCK_ADDR_BYTE] = address & BIT_MASK;
  writeBlock(global_disk_num, write_block, empty_block);
  free(empty_block);
}

void saveFiles() {
  // iterate through master file table
  // save inode block numbers to bytes in superblock
  // assumes inodes already contain info like timestamp and permissions
  int i;
  char buff[BLOCKSIZE];

  readBlock(global_disk_num, 0, buff);
  buff[BYTE_OFFSET+1] = totalFiles; // saves total number of inodes into index 4
  for(i = 1; i <= totalFiles; i++) {
    buff[BYTE_OFFSET+i+1] = master_filetable[i-1]->inode_bNum; // starts saving inodes into index 5
  }
  writeBlock(global_disk_num, 0, buff);
}

void populateFiles() {
  // iterate through inode block numbers in superblock
  // get inode block, populate master file table entries
  // fill entries with data eg time permissions names
  int i, index = 0;
  char super_buffer[BLOCKSIZE];
  char inode_buffer[BLOCKSIZE];
  char filename[BLOCKSIZE];
  file* newFile;

  readBlock(global_disk_num, 0, super_buffer);
  totalFiles = super_buffer[BYTE_OFFSET+1];
  for(i = 0; i < totalFiles; i++) {
    newFile = malloc(sizeof(file));
    master_filetable[i] = newFile;
    newFile->fd = i;
    newFile->inode_bNum = super_buffer[BYTE_OFFSET+i+2];
    newFile->numInstances = 0;

    readBlock(global_disk_num, super_buffer[BYTE_OFFSET+i+2], inode_buffer);
    while (inode_buffer[BYTE_OFFSET+index] != '\0' && index < NAME_OFFSET) {
      filename[index] = inode_buffer[BYTE_OFFSET+index];
      index++;
    }
    filename[BYTE_OFFSET+index] = '\0';
    newFile->name = filename;
    newFile->permissions = inode_buffer[BYTE_OFFSET+NAME_OFFSET+4];
    populateFileTimestamps(i, inode_buffer);
  }
}

void populateFileTimestamps(fileDescriptor fd, char* buff) {
  char* creation_stamp = malloc(TIMESTAMP_SIZE);
  char* mod_stamp = malloc(TIMESTAMP_SIZE);
  char* access_stamp = malloc(TIMESTAMP_SIZE);
  int i;

  for(i = 0; i < TIMESTAMP_SIZE; i++) {
    creation_stamp[i] = buff[CREATION_OFFSET+i];
  }
  master_filetable[fd]->creation_time = creation_stamp;

  for(i = 0; i < TIMESTAMP_SIZE; i++) {
    mod_stamp[i] = buff[MODIFICATION_OFFSET+i];
  }
  master_filetable[fd]->mod_time = mod_stamp;

  for(i = 0; i < TIMESTAMP_SIZE; i++) {
    access_stamp[i] = buff[ACCESS_OFFSET+i];
  }
  master_filetable[fd]->access_time = access_stamp;
}

void populateFreeBlocks() {
  if (free_head != NULL) {
    return;
  }
  free_head = malloc(sizeof(freeBlock));
  freeBlock* temp = free_head;
  int address = 0;
  int prev_address = 0;
  char buff[1];

  free_head->address = 0;
  free_head->next = NULL;

  char my_buff[BLOCKSIZE];

  do {
    prev_address = address;
    readBlock(global_disk_num, address, my_buff);
    address = (int) my_buff[BLOCK_ADDR_BYTE];
    addEmptyBlock(address);
  } while (address != 0 && address != prev_address);
  // clear the temporary head
  getEmptyBlock();
}
Contact GitHub API Training Shop Blog About
© 2017 GitHub, Inc. Terms Privacy Security Status Help
