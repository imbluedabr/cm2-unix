#pragma once
#include <fs/fs.h>

#define DENTRY_MAX_CHILDREN 7
#define DENTRY_CHILD_COUNT_MSK 0b111111
#define DENTRY_MOUNT_POINT_MSK (1 << 6)
#define DENTRY_LOCKED_MSK (1 << 7)
struct dentry {
    uint8_t children[DENTRY_MAX_CHILDREN];
    uint8_t packed; //uint child_count : 0-5, bool mount_point : 6, bool locked : 7
    struct inode* dir;
    struct superblock* fs;
};

#define DENTRY_MAX 20
#define DENTRY_INDEX_NIL 255
#define DENTRY_ROOT 0
extern struct dentry dentry_pool[DENTRY_MAX];

uint8_t dentry_local_lookup(uint8_t dentry_index, char* name);
uint8_t dentry_lookup(const char* path);

