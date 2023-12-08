#include <stdio.h>
#include <ostream>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstdint>
#include "mbr.h"

using namespace std;

void seekLocation(int fd, uint64_t distance){
    lseek(fd, distance, SEEK_SET);
}

uint64_t getPartAdd(int fd){
    DISK_mbr mbr;
    unsigned char *buffer;
    int buffer_return_value;

    buffer = (unsigned char *)&mbr;

    buffer_return_value = read(fd, buffer, sizeof(DISK_mbr));
    if(buffer_return_value < 0){
         fprintf(stderr, "getPartAdd: unable to read disk, return value = %d\n", buffer_return_value );
    }
    printf("The partition starts at address 0x%010x\n", mbr.pt[1].first_sector_lba * 512 + 0x400);
    return(mbr.pt[1].first_sector_lba * 512 + 0x400);
  

}

void readNTFSHeader(int fd, uint64_t partition_address){
    uint32_t count;
    SUPER_BLOCK superb;
    BLOCK blocks;
    unsigned char *buffer;
    int buffer_return_value;

    buffer = (unsigned char *)&superb;

    buffer_return_value = read(fd, buffer, sizeof(SUPER_BLOCK));
    if(buffer_return_value < 0){
         fprintf(stderr, "getPartAdd: unable to read disk, return value = %d\n", buffer_return_value );
    }
    printf("# Inodes: %d | # Blocks: %d\n", superb.inodes_count, superb.blocks_count);
    seekLocation(fd, partition_address-0x400);

    buffer = (unsigned char *)&blocks;
    for(count = 0; count < superb.blocks_count; count++){
        buffer_return_value = read(fd, buffer, sizeof(BLOCK));
        if(buffer_return_value < 0){
            fprintf(stderr, "getPartAdd: unable to read disk, return value = %d\n", buffer_return_value );
        }
        if(blocks.blocks[0] == 0x4d && blocks.blocks[1] == 0x54 && blocks.blocks[2] == 0x68 && blocks.blocks[3] == 0x64){
            printf("Found a possible header: %0x %0x %0x %0x. Located at 0x%08x, block #%d\n", blocks.blocks[0], blocks.blocks[1], blocks.blocks[2], 
            blocks.blocks[3],(partition_address - 0x400) + (count * 4096), count);
        }

    }

}

int main(int argc, char *argv[]){
    char* toolName;
    char* fileName;
    int fd;
    uint64_t partition_address;
    uint64_t MFT_address;

    fd = open(argv[1], O_RDONLY);
    partition_address = getPartAdd(fd);
    printf("The partition starts at address 0x%08x\n", partition_address);
    seekLocation(fd, partition_address);
    readNTFSHeader(fd, partition_address);
    

}
