#pragma once

#include <uapi/vfs.h>
#include <kernel/device.h>
#include <stdint.h>

struct superblock;
struct fd;
struct dentry;

#define FS_GET_INO_OFF(ID) (ID << 16)

struct romfs_inode {
    void* data;
};

struct devfs_inode {
    dev_t devno;
};

struct fatfs_inode {
    uint8_t fat_index;
};

struct inode {
    struct superblock* fs; //this is the filesystem that the inode is part of
    char name[FS_INAME_LEN + 1]; //the name of the file
    uint8_t refcount; //the amount of references exist to this inode, this includes file descriptors and dentries
    uint8_t mode; //currently only stores filetype, but i will later expand it will actual permissions and ownership
    ino_t dir; //the parent directory of this inode
    ino_t file; //the inum of this inode
    uint32_t size;
    union {
        struct romfs_inode romfs;
        struct devfs_inode devfs;
        struct fatfs_inode fatfs;
    };
};

#define INODE_TABLE_SIZE 32
extern struct inode inode_table[INODE_TABLE_SIZE];
extern uint8_t inode_free_list_size;

typedef struct {
    struct device_request* req;
    struct superblock* fs;
    struct inode* dir;
    char* fname;
} fs_lookup_t;

typedef struct {
    struct device_request* req;
    struct superblock* fs;
    struct fd* descriptor;
    uint32_t bytes_read;
    uint32_t count;
    void* buffer;
} fs_read_t;

typedef struct {
    struct device_request* req;
    struct superblock* fs;
    struct fd* descriptor;
    uint32_t bytes_written;
    uint32_t count;
    void* buffer;
} fs_write_t;

typedef struct {
    struct device_request* req;
    struct superblock* fs;
    struct fd* descriptor;
    uint32_t bytes_read;
    uint32_t count;
    void* buffer;
    uint32_t offset;
} fs_pread_t;

typedef struct {
    struct device_request* req;
    struct superblock* fs;
    struct fd* descriptor;
    uint32_t bytes_written;
    uint32_t count;
    void* buffer;
    uint32_t offset;
} fs_pwrite_t;


typedef struct {
    struct device_request* req;
    struct inode* dir;
    char* name;
} fs_dirop_t;

//TODO: the current api is not very good but i can chage it later
//these operate on file systems
struct super_ops {
    int8_t (*lookup)(fs_lookup_t* state); //lookup an inode in a dir
    int (*mount)(struct inode* mountpoint, dev_t devno, const char* args);
    int (*umount)(struct superblock* fs);
    int (*mkdir)(fs_dirop_t* state);
    int (*rmdir)(fs_dirop_t* state);
    int (*unlink)(fs_dirop_t* state);
};

//these operate on file descriptors
struct file_ops {
    int8_t (*read)(fs_read_t* state);
    int8_t (*write)(fs_write_t* state);
    int (*lstat)(struct superblock* fs, struct fd* f, struct stat* statbuff); //these are instant (i hope)
    int (*lseek)(struct superblock* fs, struct fd* f, uint32_t offset, int whence);
    int8_t (*readdir)(fs_read_t* state);
    int8_t (*pread)(fs_pread_t* state);
    int8_t (*pwrite)(fs_pwrite_t* state);
};

struct superblock {
    struct super_ops* sops;
    struct file_ops* fops;
};

struct fd {
    struct inode* file;
    uint32_t offset : 24;
    uint8_t flags : 8;
};

#define MAX_FILESYSTEM_COUNT 4
#define FS_NAME_LEN 4
#define MAX_FD 32
extern struct fd fd_table[MAX_FD];


void fs_init();
struct inode* create_inode(const char* name);
void free_inode(struct inode* i);
struct inode* get_inode_ref(struct inode* i);
int8_t lookup_dir(fs_lookup_t* state);
struct inode* lookup_ino(ino_t ino);
void register_filesystem(const char* name, struct super_ops* fs);
struct super_ops* lookup_filesystem(const char* name);
int fd_alloc();
int fd_get_free();


