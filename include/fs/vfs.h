#include <fs/fs.h>

//register a file system with a 4 letter name
void register_filesystem(const char* name, struct super_ops* fs);

//vfs api
int vfs_open(const char* path, uint8_t flags);
int vfs_read(int fd, void* buffer, uint32_t count);
int vfs_write(int fd, const void* buffer, uint32_t count);
int vfs_lstat(int fd, struct stat* statbuff);
int vfs_lseek(int fd, uint32_t offset, int whence);
void vfs_close(int fd);
int vfs_readdir(int fd, void* buffer, uint32_t count);
int vfs_pread(int fd, void* buffer, uint32_t count, uint32_t offset);
int vfs_pwrite(int fd, const void* buffer, uint32_t count, uint32_t offset);
void vfs_mount(const char* mount, const char* dev_path, const char* fs_name);
void vfs_umount(const char* mount);
int vfs_mkdir(const char* path);
int vfs_rmdir(const char* path);
int vfs_unlink(const char* path);


