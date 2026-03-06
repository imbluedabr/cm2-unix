#pragma once
#include <fs/fs.h>

struct devfs_file {
    char name[FS_INAME_LEN];
    dev_t devno;
};

#define DEVFS_MAX_FILES 16
#define DEVFS_CREATE_INO(DEV_FILENO) (DEV_FILENO + FS_GET_INO_OFF(1))

//i dont suport directories in devfs to save memory
struct devfs_superblock {
    struct superblock base;
    struct devfs_file files[DEVFS_MAX_FILES];
};

extern const struct super_ops devfs_sops;
extern const struct file_ops devfs_fops;

void devfs_init();
int8_t devfs_lookup(fs_lookup_t* state);
int devfs_mount(struct inode* mountpoint, dev_t devno, const char* args);

int8_t devfs_read(fs_read_t* state);
int8_t devfs_write(fs_write_t* state);
int8_t devfs_readdir(fs_read_t* state);

int devfs_create_handle(const char* name, dev_t devno);




