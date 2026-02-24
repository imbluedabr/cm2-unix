#include <stdint.h>
#include <uapi/syscalls.h>
#include <kernel/proc.h>

extern uint32_t syscall_args[4];

void set_read_state(fs_read_t* state, struct fd* desc, void* buffer, int count);
void block_proc();

//int open(const char* path, uint8_t flags)
void sys_open();
void sys_open_update(struct proc* process);
//int read(int fd, void* buffer, int count)
void sys_read();
void sys_read_update(struct proc* process);
//int write(int fd, void* buffer, int count)
void sys_write();
void sys_write_update(struct proc* process);
//void close(int fd)
void sys_close();
//int ioctl(int fd, int cmd, void* arg)
void sys_ioctl();
//int fstat(int fd, struct stat* buff)
void sys_fstat();
//simplified by combining stat and dirent struct
//int readdir(int fd, struct stat* buff, int count)
void sys_readdir();
void sys_readdir_update(struct proc* process);
//int getcwd(char* buff, int size)
void sys_getcwd();
//int chdir(const char* path)
void sys_chdir();
void sys_chdir_update(struct proc* process);
//void yield();
void sys_yield();
//void exit(int error_code)
void sys_exit();
//int kill(pid_t upid)
void sys_kill();
//int wait(pid_t upid)
void sys_wait();
void sys_wait_update(struct proc* process);
//int mount(const char* dev_path, const char* dir_path, const char* type)
void sys_mount();
void sys_mount_update(struct proc* process);
//int umount(const char* dir_path)
void sys_umount();
void sys_umount_update(struct proc* process);
//int sysctl(int cmd, void* buff, int count);
void sys_sysctl();

//called from a thread, switches to kernel context
int syscall(uint32_t a, uint32_t b, uint32_t c, uint32_t d);

void syscall_update();

//called by the assembly syscall function
void process_syscall();

[[noreturn]] void exit_kernel();

