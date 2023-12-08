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
    printf("The partition starts at address 0x%08x\n", mbr.pt[0].first_sector_lba * 512);
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

uint64_t calculateFileAddress(uint64_t mft_address, int file_entry){
    int decimal_address = file_entry * 1024;
    return mft_address + decimal_address;
}

void readMFTHeader(int fd, int file_entry){
    const uint32_t FILE_NAME_ATTRIBUTE = 0x00000030;

    int counter = 0;
    MFT_Entry entry;
    unsigned char *buffer;
    int buffer_return_value;
    int attribute_location;
    uint32_t attribute_size;
    uint32_t attribute_type;
    uint32_t attribute_id;
    uint32_t end_of_file = 4294967295;

    uint64_t parent_entry_number;
    uint8_t file_name_length;
    char* entry_attribute_name;

    buffer = (unsigned char *)&entry;

    buffer_return_value = read(fd, buffer, 1024);
    if(buffer_return_value < 0){
        fprintf(stderr, "Read MFT Entry, unable to read disk, return value = %d\n", buffer_return_value);
    }
    printf("Entry number: %d\n", entry.mft_record_number);
    printf("Entry in use: %s\n", (entry.flags & 1) ? "In Use" : "Deleted");
    printf("Entry is directory: %s\n", (entry.flags >> 1) ? "Directory" : "File");
    //Get the first attribute location
    attribute_location = entry.first_attr_offset - 48;
    attribute_type = (entry.attributes[attribute_location]) + (entry.attributes[attribute_location + 1] << 8) + (entry.attributes[attribute_location + 2] << 16) + (entry.attributes[attribute_location + 3] << 24);
    while(attribute_type != end_of_file && counter < 5){
        attribute_size = (entry.attributes[attribute_location+ 4]) + (entry.attributes[attribute_location + 5] << 8) + (entry.attributes[attribute_location + 6] << 16) + (entry.attributes[attribute_location + 7] << 24);
        attribute_id = (entry.attributes[attribute_location + 14]) + (entry.attributes[attribute_location + 15] << 8);
        printf("%d Attribute size: %x\n",counter+1, attribute_size);
        printf("%d Attribute type: %x\n",counter+1, attribute_type);
        printf("%d Attribute id: %x\n",counter+1, attribute_id);
        if(attribute_type == FILE_NAME_ATTRIBUTE){
            //Standard attribute is 0x18 in length, File reference is at 0x18
            parent_entry_number = (entry.attributes[attribute_location + 24]) + (entry.attributes[attribute_location + 25] << 8) + (entry.attributes[attribute_location + 26] << 16) + (entry.attributes[attribute_location + 27] << 24) + (entry.attributes[attribute_location + 28] << 32) + (entry.attributes[attribute_location + 29] << 40);
            printf("Parent Entry: %d\n", parent_entry_number);
            file_name_length = (entry.attributes[attribute_location + 88]);
            printf("File Name Length in characters: %d\n", file_name_length);
            entry_attribute_name = (char*)malloc(file_name_length * sizeof(char));
            for(int i = 0; i < file_name_length; i++){
                entry_attribute_name[i] = entry.attributes[attribute_location + 90 + (2*i)];
            }
            printf("File Name: %s\n", entry_attribute_name);
        }

        attribute_location += attribute_size;
        counter++;
        attribute_type = (entry.attributes[attribute_location]) + (entry.attributes[attribute_location + 1] << 8) + (entry.attributes[attribute_location + 2] << 16) + (entry.attributes[attribute_location + 3] << 24);
        printf("\n");
    }
    printf("Last Attribute type: %x\n",attribute_type);



}

int main(int argc, char *argv[]){
    char* toolName;
    char* fileName;
    int fd;
    int file_entry = atoi(argv[3]);
    uint64_t partition_address;
    uint64_t MFT_address;
    uint64_t file_location;

    fd = open(argv[1], O_RDONLY);
    partition_address = getPartAdd(fd);
    printf("The partition starts at address 0x%08x\n\n", partition_address);
    seekLocation(fd, partition_address);
    MFT_address = readNTFSHeader(fd);
    MFT_address += partition_address;
    printf("The byte address location of the MFT is at 0x%08x\n\n", MFT_address);
    seekLocation(fd, MFT_address);
    file_location = calculateFileAddress(MFT_address, file_entry);
    printf("File Address is 0x%08x\n\n", file_location);
    seekLocation(fd, file_location);
    readMFTHeader(fd, file_entry);


}
