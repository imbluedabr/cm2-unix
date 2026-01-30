#include <stddef.h>
#include <fs/fs.h>
#include <fs/dcache.h>
#include <kernel/vfs.h>
#include <lib/stdlib.h>

//this is just a private type
#define FS_NAME_LEN 4
struct fs_tuple {
    struct super_ops* fs;
    char name[FS_NAME_LEN];
};

//this maps names to fileystem types
#define MAX_FILESYSTEM_COUNT 4
struct fs_tuple filesystem_registry[MAX_FILESYSTEM_COUNT];

#define MAX_FD 32
struct fd fd_table[MAX_FD];

void register_filesystem(const char* name, struct super_ops* fs)
{
    for (int i = 0; i < MAX_FILESYSTEM_COUNT; i++) {
        if (filesystem_registry[i].fs == NULL) {
            filesystem_registry[i].fs = fs;
            strncpy(filesystem_registry[i].name, (char*) name, FS_NAME_LEN);
            break;
        }
    }
}


//maybe there is a faster way of allocating these?
int fd_alloc()
{
    for (int i = 0; i < MAX_FD; i++) {
        if (fd_table[i].file == NULL) {
            return i;
        }
    }
    return -1;
}



int vfs_open(const char* path, uint8_t flags)
{
    /* work in progress
    
    struct dentry* file_dentry = &dentry_pool[dentry_lookup(path)];
    
    int file_descriptor = fd_alloc();
    struct fd* file_ptr = &fd_table[file_descriptor];
    file_ptr->file = file_dentry->dir;
    file_ptr->offset = 0;
    file_ptr->flags = flags;
    */
    return -1;
}

int vfs_read(int fd, void* buffer, uint32_t count)
{

}

int vfs_write(int fd, const void* buffer, uint32_t count)
{

}

int vfs_lstat(int fd, struct stat* statbuff)
{

}

int vfs_lseek(int fd, uint32_t offset, int whence)
{

}

void vfs_close(int fd)
{

}

int vfs_readdir(int fd, void* buffer, uint32_t count)
{

}

int vfs_pread(int fd, void* buffer, uint32_t count, uint32_t offset)
{

}

int vfs_pwrite(int fd, const void* buffer, uint32_t count, uint32_t offset)
{

}

void vfs_mount(const char* mount, const char* dev_path, const char* name)
{

}

void vfs_umount(const char* mount)
{

}

//we can also choose to only implement the *at versions of these syscalls since they are simpeler and more flexible, e.g mkdirat, rmdirat, unlinkat, these take a dirfd and a name
int vfs_mkdir(const char* path)
{

}

int vfs_rmdir(const char* path)
{

}

int vfs_unlink(const char* path)
{

}

