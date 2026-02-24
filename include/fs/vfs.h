#pragma once
#include <fs/fs.h>
#include <kernel/device.h>

typedef struct {
    char path_cpy[FS_PATH_LEN];
    uint8_t path_index;
    const char* path;
    fs_lookup_t fs_state;
} path_walk_t;

typedef struct {
    path_walk_t path;
    struct super_ops* fs;
    dev_t devno;
} vfs_mount_t;

extern struct inode rootfs;

void walk_path_init(path_walk_t* state, const char* path);
int8_t walk_path(path_walk_t* state);

int mount_root(const char* fs_name, dev_t devno);
int mount_devfs(const char* fs_name);
int8_t mount_init(vfs_mount_t* state, const char* path, const char* fs_name, dev_t devno);
int8_t mount_update(vfs_mount_t* state);
void vfs_close(int fileno);

