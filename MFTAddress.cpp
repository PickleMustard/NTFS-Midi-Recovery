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
    //printf("The partition starts at address 0x%08x\n", mbr.pt[1].first_sector_lba * 512);
    printf("The partition starts at address 0x%010x\n", mbr.pt[1].first_sector_lba * 512);
    return(mbr.pt[0].first_sector_lba * 512);
  

}

uint64_t readNTFSHeader(int fd){
    NTFS_header_entry ntfs;
    unsigned char *buffer;
    int buffer_return_value;

    buffer = (unsigned char *)&ntfs;

    buffer_return_value = read(fd, buffer, sizeof(NTFS_header_entry));
    if(buffer_return_value < 0){
         fprintf(stderr, "getPartAdd: unable to read disk, return value = %d\n", buffer_return_value );
    }
    printf("Sector Size: %d\n", ntfs.bytes_per_sector);
    printf("Sectors Per Cluster: %d\n", ntfs.sectors_per_cluster);
    printf("Logical Cluster: 0x%08x\n", ntfs.MFT_logical_cluster);
    printf("Byte Address: 0x%08x\n", ntfs.bytes_per_sector * ntfs.sectors_per_cluster * ntfs.MFT_logical_cluster);
    return(ntfs.bytes_per_sector * ntfs.sectors_per_cluster * ntfs.MFT_logical_cluster);
  

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
    MFT_address = readNTFSHeader(fd);
    MFT_address += partition_address;
    printf("The byte address location of the MFT is at 0x%08x\n", MFT_address);
    

}
