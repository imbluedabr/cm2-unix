#pragma once
#include <fs/fs.h>


struct fatfs_file {
    char name[FS_INAME_LEN + 1];
    uint8_t fat_index;
    uint8_t mode;
    uint16_t size;
    uint16_t reserved;
};


#define FATFS_SECTOR_SIZE 256
#define FATFS_DIR_LEN (FATFS_SECTOR_SIZE/sizeof(struct fatfs_file))
#define FATFS_FAT_START 0
#define FATFS_DATA_START 512

#define CALC_FADRES(INDEX) (INDEX + FATFS_FAT_START)
#define CALC_SADRES(INDEX) (INDEX*FATFS_SECTOR_SIZE + FATFS_DATA_START)

#define FATFS_SECTOR_END 0xFF
#define FATFS_SECTOR_FREE 0xFE

#define FATFS_CALC_INO(INDEX) (FS_GET_INO_OFF(3) + INDEX)

struct fatfs_superblock {
    struct superblock base;
    uint8_t fat_cache[256];
    struct fatfs_file dir_cache[FATFS_DIR_LEN];
    dev_t devno;
    struct device* dev;
};

extern const struct super_ops fatfs_sops;
extern const struct file_ops fartfs_fops;



int8_t fatfs_lookup(fs_lookup_t* state);

int fatfs_mount(struct inode* mountpoint, dev_t devno, const char* args);

int8_t fatfs_read(fs_read_t* state);

int8_t fatfs_write(fs_write_t* state);

int8_t fatfs_readdir(fs_read_t* state);

