#pragma once

#include "device.h"
#include <stdint.h>


struct superblock;
struct fd;
struct dentry;


#define FS_INAME_LEN 8
#define FS_PATH_LEN (FS_INAME_LEN*10)
struct inode {
    struct superblock* fs; //this is the filesystem that the inode is part of
    char name[FS_INAME_LEN]; //the name of the file
    uint8_t refcount; //the amount of references exist to this inode, this includes file descriptors and dentries
    uint8_t mode; //currently only stores filetype, but i will later expand it will actual permissions and ownership
};

struct stat {
    uint32_t size;
    uint32_t time;
    uint8_t mode;
    dev_t dev;
};

//these operate on file systems
struct super_ops {
    struct inode* (*lookup)(struct superblock* fs, struct inode* dir, char* name); //lookup an inode in a dir
    int (*release)(struct superblock* fs, struct inode* i); //this decrements the refcount and eventually free's the inode
    struct superblock* (*mount)(struct device* dev, char* args);
    int (*umount)(struct superblock* fs);
    int (*mkdir)(struct superblock* fs, struct inode* dir, char* name);
    int (*rmdir)(struct superblock* fs, struct inode* dir, char* name);
};

//these operate on file descriptors
struct file_ops {
    int (*read)(struct superblock* fs, struct fd* f, void* buffer, uint32_t count);
    int (*write)(struct superblock* fs, struct fd* f, void* buffer, uint32_t count);
    int (*lstat)(struct superblock* fs, struct fd* f, struct stat* statbuff);
    int (*lseek)(struct superblock* fs, struct fd* f, uint32_t offset, int whence);
    void (*close)(struct superblock* fs, struct fd* f);
    int (*readdir)(struct superblock* fs, struct fd* f, void* buffer, uint32_t count);
    int (*pread)(struct superblock* fs, struct fd* f, void* buffer, uint32_t count, uint32_t offset);
    int (*pwrite)(struct super_ops* fs, struct fd* f, void* biffer, uint32_t count, uint32_t offset);
};

struct superblock {
    struct super_ops* sops;
    struct file_ops* fops;
};


struct fd {
    struct inode* file;
    uint32_t offset; //packed variable, contains
};


