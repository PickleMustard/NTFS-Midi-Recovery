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
    const uint32_t DATA_RUN_ATTRIBUTE = 0x00000080;
    const uint8_t END_OF_DATA_RUN = 0x00;

    int counter = 0;
    MFT_Entry entry;
    unsigned char *buffer;
    int buffer_return_value;
    int attribute_location;
    uint32_t attribute_size;
    uint32_t attribute_type;
    uint32_t attribute_id;
    uint32_t end_of_file = 4294967295;

    uint8_t non_resident;
    uint8_t data_run_size;
    uint8_t data_run_length;
    uint64_t data_run_length_literal;
    int32_t data_run_offset_signed;
    uint8_t data_run_offset;
    uint32_t data_run_offset_literal;
    int data_run_location;
    bool negative;

    uint64_t parent_entry_number;
    uint64_t real_allocated_size;
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
    while(attribute_type != end_of_file){
        attribute_size = (entry.attributes[attribute_location+ 4]) + (entry.attributes[attribute_location + 5] << 8) + (entry.attributes[attribute_location + 6] << 16) + (entry.attributes[attribute_location + 7] << 24);
        attribute_id = (entry.attributes[attribute_location + 14]) + (entry.attributes[attribute_location + 15] << 8);
        printf("%d Attribute size: %x\n",counter+1, attribute_size);
        printf("%d Attribute type: %x\n",counter+1, attribute_type);
        printf("%d Attribute id: %x\n",counter+1, attribute_id);
        if(attribute_type == FILE_NAME_ATTRIBUTE){
            //Standard attribute is 0x18 in length, File reference is at 0x18
            parent_entry_number = ((uint64_t)(entry.attributes[attribute_location + 24]))
                                | ((uint64_t)(entry.attributes[attribute_location + 25]) << 8) 
                                | ((uint64_t)(entry.attributes[attribute_location + 26]) << 16) 
                                | ((uint64_t)(entry.attributes[attribute_location + 27]) << 24) 
                                | ((uint64_t)(entry.attributes[attribute_location + 28]) << 32) 
                                | ((uint64_t)(entry.attributes[attribute_location + 29]) << 40);
            printf("Parent Entry: %d\n", parent_entry_number);
            file_name_length = (entry.attributes[attribute_location + 88]);
            printf("File Name Length in characters: %d\n", file_name_length);
            entry_attribute_name = (char*)malloc(file_name_length * sizeof(char));
            for(int i = 0; i < file_name_length; i++){
                entry_attribute_name[i] = entry.attributes[attribute_location + 90 + (2*i)];
            }
            printf("File Name: %s\n", entry_attribute_name);
        } else if(attribute_type == DATA_RUN_ATTRIBUTE){
            //Print out the Resident / Non-resident flag | At the 9nth byte
            //Size of the file from the DATA attribute | 
            //Data run pairs
            real_allocated_size = (entry.attributes[attribute_location + 48]) 
                                | ((uint64_t)(entry.attributes[attribute_location + 49]) << 8) 
                                | ((uint64_t)(entry.attributes[attribute_location + 50]) << 16) 
                                | ((uint64_t)(entry.attributes[attribute_location + 51]) << 24)
                                | ((uint64_t)(entry.attributes[attribute_location + 52]) << 32)
                                | ((uint64_t)(entry.attributes[attribute_location + 53]) << 40)
                                | ((uint64_t)(entry.attributes[attribute_location + 54]) << 48)
                                | ((uint64_t)(entry.attributes[attribute_location + 55]) << 56);

            non_resident = entry.attributes[attribute_location + 8];

            printf("\nNon-Resident Data Run: %d\n", non_resident);
            printf("Real Size of file: %d\n", real_allocated_size);
            if(non_resident){
                data_run_location = attribute_location + 64;
                data_run_size = (entry.attributes[data_run_location] & 15) + (entry.attributes[data_run_location] >> 4);
                while(entry.attributes[data_run_location] != END_OF_DATA_RUN){
                    negative = false;
                    data_run_length_literal = 0;
                    data_run_offset_literal = 0;
                    data_run_length = (entry.attributes[data_run_location] & 15);
                    data_run_offset = (entry.attributes[data_run_location] >> 4);

                    printf("\nData Run Size: %d\n", data_run_size);
                    printf("Length # of Bytes: %d\n", data_run_length);
                    printf("Offset # of Bytes: %d\n", data_run_offset);

                    //length
                    for(int i = 0; i < data_run_length; i++){
                        data_run_length_literal += ((uint64_t)(entry.attributes[data_run_location + 1 + i]) << (i*8));
                    }

                    //offset
                    for(int i = 0; i < data_run_offset; i++){
                        data_run_offset_literal += ((uint32_t)(entry.attributes[data_run_location + data_run_length + 1 + i]) << (i*8));
                    }
                    data_run_offset_signed = data_run_offset_literal;

                    printf("This: %0x\n", entry.attributes[data_run_location + data_run_length + data_run_offset]);
                    if(entry.attributes[data_run_location + data_run_length + data_run_offset] & 0x80){
                        printf("Negative\n");
                        for(int i = data_run_offset; i < 4; i++){
                           data_run_offset_literal += ((uint32_t)(0xff) << data_run_offset*8);
                        }
                        data_run_offset_signed = data_run_offset_literal;
                    }

                    printf("Data Run Literal: %d\n", data_run_length_literal);
                    printf("Data Offset Literal: %d\n", data_run_offset_signed);
                    
                    data_run_location += data_run_size + 1;
                    data_run_size = (entry.attributes[data_run_location] & 15) + (entry.attributes[data_run_location] >> 4);
                }
            }

            

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
