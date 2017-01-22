#include <stdint.h>
#include <stdio.h>
//#include <ext2_fs.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

//returns true if block pointed at by bitmap entry specified by parameters is free. false if not
int isFree(char * map, int byteNum, int bitNum){
  char * byte = map + byteNum;
  int mask = (0x1 << bitNum);
  return !((*byte) & mask);
}

struct dirInode_t { 
  int i_num;
  int numBlocks;
  uint32_t i_block[15];
};

int processIndirect(uint32_t* indirectPtr, int blockSize, int containingBlock, FILE* csv);

int main(){
  
  ///////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////         SUPERBLOCK        ///////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////
  
  //Allocate the superblock
  char * superblock = malloc(1024);
  int fs = open("disk-image", O_RDONLY);
  pread(fs, superblock, 1024, 1024);
  
  //magic number
  uint16_t * magicnum = (uint16_t *) (superblock + 56);
  
  //inodeCount
  uint32_t * inodeCount = (uint32_t *) (superblock);
  
  //blockCount
  uint32_t * blockCount = (uint32_t *) (superblock + 4);
  
  //blockSize
  uint32_t * log_blockSize = (uint32_t *) (superblock + 24); //TODO: Computations
  int blockSize = 1024 << *log_blockSize;
  
  //fragSize
  uint32_t * log_fragSize = (uint32_t *) (superblock + 28); //TODO: Computations
  int fragSize;
  if (*log_fragSize >= 0)
    fragSize = 1024 << *log_fragSize;
  else
    fragSize = 1024 >> -(*log_fragSize);
  
  
  //blocksPerGroup
  uint32_t * blocksPerGroup = (uint32_t *) (superblock + 32); 
  
  //inodesPerGroup
  uint32_t * inodesPerGroup = (uint32_t *) (superblock + 40);
  
  //fragsPerGroup
  uint32_t * fragsPerGroup = (uint32_t *) (superblock + 36);  
  
  //firstDataBlock
  uint32_t * firstDataBlock = (uint32_t *) (superblock + 20);
  
  //make super.csv
  FILE * fssuper = fopen("super.csv", "w+");
  fprintf(fssuper, "%x,%d,%d,%d,%d,%d,%d,%d,%d", *magicnum, *inodeCount, *blockCount, blockSize, fragSize, *blocksPerGroup, *inodesPerGroup, *fragsPerGroup, *firstDataBlock);
  
  
  ///////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////         GROUP        //////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////
  
  
  //Allocate the group descriptor table
  char * gdt = malloc(blockSize);
  if (blockSize == 1024)
    pread(fs, gdt, blockSize, 2048);
  else
    pread(fs, gdt, blockSize, blockSize);
  
  
  //make group.csv
  FILE * fsgroup = fopen("group.csv", "w+");
  
  int numGroups;
  if(*blockCount % *blocksPerGroup == 0)
    numGroups = *blockCount / *blocksPerGroup;
  else
    numGroups = *blockCount / *blocksPerGroup + 1;
  
  int blockBitmaps[numGroups];
  int inodeBitmaps[numGroups];
  int inodeTables[numGroups];
  
  //for each group 
  for(int i=0; i < numGroups; i++){  
    //numberOfContainedBlocks
    // uint16_t* block_group_nr = (uint16_t *) (superblock + 90); //figure out which block group this super block is in
    int numberOfContainedBlocks; 
    if (i != numGroups-1) //TODO: IF NOT LAST GROUP
      numberOfContainedBlocks = *blocksPerGroup;
    else //else if last block group
      numberOfContainedBlocks = *blockCount % *blocksPerGroup;
    
    //numberOfFreeBlocks
    uint16_t * numberOfFreeBlocks = (uint16_t *) (gdt + 12);
    
    //numberOfFreeInodes
    uint16_t * numberOfFreeInodes = (uint16_t *) (gdt + 14);
    
    //numberOfDirectories
    uint16_t * numberOfDirectories = (uint16_t *) (gdt + 16);
    
    //freeInodeBitmapBlock
    uint32_t * freeInodeBitmapBlock = (uint32_t *) (gdt + 4);
    
    //freeBlockBitmapBlock
    uint32_t * freeBlockBitmapBlock = (uint32_t *) (gdt);
    
    //inodeTableStartBlock
    uint32_t * inodeTableStartBlock = (uint32_t *) (gdt + 8);
    
    fprintf(fsgroup, "%d,%d,%d,%d,%x,%x,%x\n", numberOfContainedBlocks, *numberOfFreeBlocks, *numberOfFreeInodes, *numberOfDirectories, *freeInodeBitmapBlock, *freeBlockBitmapBlock, *inodeTableStartBlock);
    
    blockBitmaps[i] = (*freeBlockBitmapBlock);
    inodeBitmaps[i] = (*freeInodeBitmapBlock);
    inodeTables[i] = (*inodeTableStartBlock);
    
    gdt += 32;
  }
  
  ///////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////         BITMAP        /////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////
  
  //make bitmap.csv
  FILE * fsbitmap = fopen("bitmap.csv", "w+");
  
  //allocated inodes array
  int allocatedInodes[(*inodeCount)];
  int size_ai = 0;
  
  //for every group
  for (int i = 0; i < numGroups; i++){
    //Block Bitmap
    uint32_t blockNumberOfMap = (uint32_t) blockBitmaps[i];
    char * bitMap = malloc(blockSize);
    int mapOffset = blockSize*blockNumberOfMap;
    pread(fs, bitMap, blockSize, mapOffset);
    int freeBlocks[(*blockCount)];
    int size_fb = 0;
    
    for(int k=0; k < (*blocksPerGroup)/8; k++){ //for each byte
      for(int j = 0; j < 8; j++){ //for each bit in byte
        if(isFree(bitMap, k, j)){
          freeBlocks[size_fb] = i*(*blocksPerGroup) + k*8 + j + 1;
          fprintf(fsbitmap, "%x,%d\n", blockNumberOfMap, freeBlocks[size_fb]);
          size_fb++;
        }
      }
    }
    
    //inode Bitmap
    uint32_t blockNumberOfiMap = (uint32_t) inodeBitmaps[i];
    char * iMap = malloc(blockSize);
    int iMapOffset = blockSize*blockNumberOfiMap;
    pread(fs, iMap, blockSize, iMapOffset);
    int freeInodes[(*inodeCount)];
    int size_fi = 0;
    
    for(int k=0; k < (*inodesPerGroup)/8; k++){ //for each byte
      for(int j = 0; j < 8; j++){ //for each bit in byte
        if(isFree(iMap, k, j)){
          freeInodes[size_fi] = i*(*inodesPerGroup) + k*8 + j + 1;
          fprintf(fsbitmap, "%x,%d\n", blockNumberOfiMap, freeInodes[size_fi]);
          size_fi++;
        }
        else{
          allocatedInodes[size_ai] = i*(*inodesPerGroup) + k*8 + j + 1;
          size_ai++;
        }
      }
    }
  }
  
  
  
  
  
  ///////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////
  
  //make inode.csv
  FILE * inode_csv = fopen("inode.csv", "w+");
  char* inode = malloc(128);
  struct dirInode_t dirInodes[size_ai];
  int size_dir = 0;
  
  for(int i=0; i < size_ai; i++){
    int block_group = (allocatedInodes[i] - 1) / *inodesPerGroup;
    int inode_index = (allocatedInodes[i] - 1) % *inodesPerGroup;
    int offset = inodeTables[block_group]*blockSize+inode_index*128;
    pread(fs, inode, 128, offset);
    
    char file_type;
    uint16_t* i_mode = (uint16_t*) inode;    // can also get the file type from this. Do this by looking at the leftmost byte
    uint16_t* i_uid = (uint16_t*) (inode + 2);
    uint16_t* i_gid = (uint16_t*) (inode + 24);
    uint16_t* i_link_count = (uint16_t*) (inode + 26);
    uint32_t* i_ctime = (uint32_t*) (inode + 12);
    uint32_t* i_mtime = (uint32_t*) (inode + 16);
    uint32_t* i_atime = (uint32_t*) (inode + 8);
    uint32_t* i_size = (uint32_t*) (inode + 4);
    uint32_t* i_blocks = (uint32_t*) (inode + 28);
    uint32_t* i_block = (uint32_t*) (inode + 40);
    
    //new end
    int mask = 0xF000;
    if((mask&(*i_mode)) == 0xA000)
      file_type = 's';
    else if((mask&(*i_mode)) == 0x8000)
      file_type = 'f';
    else if((mask&(*i_mode)) == 0x4000)
      file_type = 'd';
    else
      file_type = '?';
    
    //for directories part
    if (file_type == 'd'){  
      dirInodes[size_dir].i_num = allocatedInodes[i];
      dirInodes[size_dir].numBlocks = *i_blocks * 512 / blockSize;
      for(int p = 0; p < 15; p++){
        dirInodes[size_dir].i_block[p] = *(i_block+p);
      }
      size_dir++;
    }
    
    fprintf(inode_csv, "%d,%c,%o,%d,%d,%d,%x,%x,%x,%d,%d", allocatedInodes[i], file_type, *i_mode, *i_uid, *i_gid, *i_link_count, 
            *i_ctime, *i_mtime, *i_atime, *i_size, *i_blocks * 512 / blockSize);
    for(int i = 0; i < 15; i++){
      fprintf(inode_csv,",%x", *i_block);
      i_block++;
    }
    fprintf(inode_csv,"\n");
    
    
  }
  
  
  
  
  ///////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////         DIRECTORIES        ///////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////
  
  //make directory.csv
  FILE * directory_csv = fopen("directory.csv", "w+");
  
  char* directory = malloc(blockSize); 
  
  //for all directory inodes
  for (int i = 0; i < size_dir; i++){
    //for all blocks in that specific directory
    int entryNum = 0;
    for (int j = 0; j < dirInodes[i].numBlocks; j++){
      uint16_t temp = 0;
      while(1){
        pread(fs, directory, blockSize, dirInodes[i].i_block[j]*blockSize+temp);
        uint32_t * inode = (uint32_t*) (directory);
        uint16_t * rec_len = (uint16_t*) (directory+4);
        temp += *rec_len;char * name_len = (directory+6);
        //char * file_type = (directory+7);
        char name[255];
        int size_name = 0;
        for (int l = 0; l < *name_len; l++){
          name[size_name] = *(directory+8+l);
          size_name++;
        }
        name[size_name] = '\0';
        if (*inode != 0)
          fprintf(directory_csv, "%d,%d,%d,%d,%d,\"%s\"\n", dirInodes[i].i_num, entryNum, *rec_len, *name_len, *inode, name);
        entryNum++;
        if (temp >= blockSize)
          break;
      }
    } 
  }
  
  
  
  ///////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////         INDIRECT BLOCK ENTRY        ///////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////
  
  //make indirect.csv
  FILE * indirect_csv = fopen("indirect.csv", "w+");
  
  uint32_t* indirectBlock = malloc(blockSize);
  uint32_t* doubleIndirectBlock = malloc(blockSize); 
  uint32_t* tripleIndirectBlock = malloc(blockSize);
  
  //for all inodes
  for (int i = 0; i < size_ai; i++){
    //this gets inode
    int block_group = (allocatedInodes[i] - 1) / *inodesPerGroup;
    int inode_index = (allocatedInodes[i] - 1) % *inodesPerGroup;
    int offset = inodeTables[block_group]*blockSize+inode_index*128;
    pread(fs, inode, 128, offset);
    
    //this checks if any indirect blocks exist for inode
    uint32_t* i_blocks = (uint32_t*) (inode + 28);
    uint32_t* i_block = (uint32_t*) (inode + 40);
    int numBlocks = *i_blocks * 512 / blockSize;
    
    //if there are indirect blocks
    if (numBlocks > 12){
      if(i_block[12] != 0){
        pread(fs, indirectBlock, blockSize, i_block[12]*blockSize);
        if(indirectBlock != 0){
          processIndirect(indirectBlock, blockSize, i_block[12], indirect_csv);
        }
      }
      if(i_block[13] != 0){
        pread(fs, doubleIndirectBlock, blockSize, i_block[13]*blockSize);
        if(doubleIndirectBlock != 0){
          int temp = processIndirect(doubleIndirectBlock, blockSize, i_block[13], indirect_csv);
          for(int j = 0; j < temp; j++){
            pread(fs, indirectBlock, blockSize, doubleIndirectBlock[j]*blockSize);
            if(indirectBlock != 0){
              processIndirect(indirectBlock, blockSize, doubleIndirectBlock[j], indirect_csv);
            }     
          }
        }
      }
      if(i_block[14] != 0){
        pread(fs, tripleIndirectBlock, blockSize, i_block[14]*blockSize);
        if(tripleIndirectBlock != 0){
          int temp1 = processIndirect(tripleIndirectBlock, blockSize, i_block[14], indirect_csv);
          for(int j = 0; j < temp1; j++){
            pread(fs, doubleIndirectBlock, blockSize, tripleIndirectBlock[j]*blockSize);
            if(doubleIndirectBlock != 0){
              int temp = processIndirect(doubleIndirectBlock, blockSize, i_block[14], indirect_csv);
              for(int k = 0; k < temp; k++){
                pread(fs, indirectBlock, blockSize, doubleIndirectBlock[k]*blockSize);
                if(indirectBlock != 0){
                  processIndirect(indirectBlock, blockSize, doubleIndirectBlock[k], indirect_csv);
                } 
              }
            }
          }
        }
      }
    }
  }
}
int processIndirect(uint32_t* indirectPtr, int blockSize, int containingBlock, FILE* csv){
  if(indirectPtr == 0)
    return 0;
  int counter = 0;
  int numPointers = 256; // An indirect pointer points to 256 data blocks
  for(int i=0; i<numPointers; i++){
    uint32_t data = indirectPtr[i];
    if(data == 0)
      break;
    else
      fprintf(csv, "%x,%d,%x\n", containingBlock, i, data);
    counter++;
  }
  return counter;
}