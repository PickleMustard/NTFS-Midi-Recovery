#ifndef __MBR__

#define __MBR__
#include <stdint.h>

typedef struct ntfs_header_entry {
    uint8_t ntfs_descriptor[11];


    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint8_t reserved[7];
    uint8_t media_descriptor;
    uint16_t reserved_additional;
    uint8_t unused_space[8];
    uint32_t more_reserved;
    uint64_t total_sectors;
    uint32_t reserved_after_sectors;
    uint64_t MFT_logical_cluster;
    uint64_t MFT_copy_logical_cluster;
    uint8_t MFT_clusters_per_record;
    uint8_t unused_space_two[3];
    uint8_t clusters_per_index_buffer;
    uint8_t unused_space_three[3];
    uint64_t volume_serial_num;
    uint8_t unused_space_four[4];

} __attribute__((packed)) NTFS_header_entry;

typedef struct mbr_partition_table_entry
{
    uint8_t status;
    uint8_t start_chs[3];
    uint8_t partition_type;
    uint8_t end_chs[3];
    uint32_t first_sector_lba;
    uint32_t sector_count;
} MBR_partition_table_entry;


typedef struct disk_mbr
{
    uint8_t code[440];
    uint32_t disk_signature;
    uint16_t reserved;
    MBR_partition_table_entry pt[4];
    uint8_t signature[2];
} __attribute__((packed)) DISK_mbr;

typedef struct mft_header{
    uint32_t file_indicator;
    uint16_t update_sequence_offset;
    uint16_t num_fixup_array_entries;
    uint64_t seq_num_logfile;
    uint16_t seq_num;
    uint16_t hard_link;
    uint16_t first_attr_offset;
    uint16_t flags;
    uint32_t entry_used_size;
    uint32_t entry_allocated_size;
    uint64_t base_file_reference;
    uint16_t next_attr;
    uint16_t alignment;
    uint32_t mft_record_number;
    uint8_t attributes[994];
} __attribute__((packed)) MFT_Entry;

typedef struct file_data_attributes{
    int num_of_data_attributes;
    uint64_t starting_address;
} FILE_DATA_ATTR;

typedef struct super_block{
    uint32_t inodes_count;
    uint32_t blocks_count;
    uint32_t r_blocks_count;
    uint32_t free_block_count;
    uint32_t free_inode_count;
    uint32_t first_data_block;
    uint32_t log_block_size;
    uint32_t log_cluster_size;
    uint32_t blocks_per_group;
    uint32_t clusters_per_group;
    uint32_t inodes_per_group;
    uint32_t s_mtime;
    uint32_t s_wtime;
    uint16_t s_mount_count;
    uint16_t max_mount_count;
    uint16_t magic_number;
}__attribute__((packed)) SUPER_BLOCK;

typedef struct block{
    uint8_t blocks[4096];
} BLOCK;
#endif
