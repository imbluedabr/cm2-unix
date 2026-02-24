#include <kernel/globals.h>
#include <kernel/syscall.h>
#include <kernel/device.h>
#include <kernel/panic.h>
#include <kernel/proc.h>
#include <kernel/exec.h>
#include <lib/kprint.h>
#include <lib/stdlib.h>
#include <lib/alloc.h>
#include <fs/vfs.h>
#include <stddef.h>


uint32_t syscall_args[4]; //arguments registers, we only need a0 to a3 right now

void (*syscall_setup_table[])() = {
    &sys_open,
    &sys_read,
    &sys_write,
    &sys_close,
    &sys_ioctl,
    &sys_fstat,
    &sys_readdir,
    &sys_getcwd,
    &sys_chdir,
    &sys_yield,
    &sys_exec,
    &sys_exit,
    &sys_kill,
    &sys_wait,
    &sys_mount,
    &sys_umount,
    &sys_sysctl
};

void (*syscall_update_table[])(struct proc* process) = {
    &sys_open_update,
    &sys_read_update,
    &sys_write_update,
    NULL,
    NULL,
    NULL,
    &sys_readdir_update,
    NULL,
    &sys_chdir_update,
    NULL,
    &sys_exec_update,
    NULL,
    NULL,
    &sys_wait_update,
    &sys_mount_update,
    &sys_umount_update,
    NULL
};

#define SYSCALL_COUNT sizeof(syscall_setup_table)/sizeof(void*)

void set_read_state(fs_read_t* state, struct fd* desc, void* buffer, int count)
{
    state->buffer = buffer;
    state->count = count;
    state->descriptor = desc;
    state->fs = desc->file->fs;
    state->req = NULL;
    state->bytes_read = 0;
}

void block_proc()
{
    current_process->state = BLOCKED;
    current_process->syscall_state = SYSCALL_STATE_BEGIN;
}

//syscall from user process

// int open(const char* path, uint8_t flags)
void sys_open()
{
    struct proc* process = current_process;
    walk_path_init(&process->open_state.walker, (const char*) syscall_args[1]);
    
    //block the process
    block_proc();
}

void sys_open_update(struct proc* process)
{
    int8_t rt = walk_path(&process->open_state.walker);
    if (rt < 0) { //directory not found
        proc_resume(process, -1);
    } else if (rt == 1) { //we walked the entire path
        uint8_t new_fd = fd_alloc();
        struct fd* fd_p = &fd_table[new_fd];
        struct inode* i = process->open_state.walker.fs_state.dir;
        i->refcount++;
        fd_p->file = i;
        fd_p->flags = 0;
        fd_p->offset = 0;
        
        uint8_t fdnum = proc_alloc_fd(process);
        if (fdnum == PROC_FILE_NIL) { 
            proc_resume(process, -1);
            return;
        }
        
        process->open_files[fdnum] = new_fd;
        proc_resume(process, fdnum);
    }
}

//int read(int fd, void* buffer, uint32_t count)
void sys_read()
{
    struct proc* process = current_process;
    struct fd* descriptor = proc_get_fd(syscall_args[1]);
    if (descriptor == NULL || !FS_IS_A_FILE(descriptor->file->mode)) {
        process->return_value = -1;
        return;
    }
    
    set_read_state(&process->read_state.fs, descriptor, (void*) syscall_args[2], syscall_args[3]);

    block_proc();
}

void sys_read_update(struct proc* process)
{
    int8_t (*read)(fs_read_t* state) = process->read_state.fs.fs->fops->read;
    if (read == NULL) {
        proc_resume(process, -1);
        return;
    }
    
    if (read(&process->read_state.fs) != 0) {
        proc_resume(process, process->read_state.fs.bytes_read);
    }

}

void sys_write()
{
    struct proc* process = current_process;
    struct fd* descriptor = proc_get_fd(syscall_args[1]);
    if (descriptor == NULL || !FS_IS_A_FILE(descriptor->file->mode)) {
        process->return_value = -1;
        return;
    }
    
    set_read_state((fs_read_t*) &process->write_state.fs, descriptor, (void*) syscall_args[2], syscall_args[3]);

    block_proc();
}

void sys_write_update(struct proc* process)
{
    int8_t (*write)(fs_write_t* state) = process->write_state.fs.fs->fops->write;
    if (write == NULL) {
        proc_resume(process, -1);
        return;
    }
    
    if (write(&process->write_state.fs) != 0) {
        proc_resume(process, process->write_state.fs.bytes_written);
    }
}

void sys_close()
{
    vfs_close(syscall_args[1]);
}

//int ioctl(int fd, int cmd, void* arg)
void sys_ioctl()
{
    struct fd* descriptor = proc_get_fd(syscall_args[1]);
    if (descriptor == NULL) {
        current_process->return_value = -1;
        return;
    }
    struct device* dev = device_lookup(descriptor->file->devfs.devno);
    current_process->return_value = dev->ops->ioctl(dev, syscall_args[2], (void*) syscall_args[3]);
}

//int fstat(int fd, struct stat* buff)
void sys_fstat()
{
    struct stat* buff = (struct stat*) syscall_args[1];
    struct fd* descriptor = proc_get_fd(syscall_args[0]);
    if (descriptor == NULL) {
        current_process->return_value = -1;
        return;
    }
    struct inode* i = descriptor->file;

    buff->d_ino = i->file;
    buff->mode = i->mode;
    strlcpy(buff->name, i->name, FS_INAME_LEN - 1);
    //TODO: call the fs fstat implementation
    current_process->return_value = 0;
}

void sys_readdir()
{
    struct proc* process = current_process;
    struct fd* descriptor = proc_get_fd(syscall_args[1]);
    if (descriptor == NULL || FS_IS_A_FILE(descriptor->file->mode)) {
        process->return_value = -1;
        return;
    }
    
    set_read_state(&process->read_state.fs, descriptor, (void*) syscall_args[2], syscall_args[3]);

    block_proc();
}

void sys_readdir_update(struct proc* process)
{
    int8_t (*readdir)(fs_read_t* state) = process->read_state.fs.fs->fops->readdir;
    if (readdir == NULL) {
        proc_resume(process, 0);
        return;
    }

    int8_t rt = readdir(&process->read_state.fs);

    if (rt != 0) {
        proc_resume(process, process->read_state.fs.bytes_read);
    }
}

//int getcwd(char* buff, int size)
void sys_getcwd()
{

}

void sys_chdir()
{

}

void sys_chdir_update(struct proc* process)
{

}

void sys_yield()
{
    //yield does nothing lol
}

//void exit(int return_code)
void sys_exit()
{
    current_process->state = DEAD;
    current_process->return_value = syscall_args[1];
}

//int kill(pid_t upid)
void sys_kill()
{
    pid_t upid = syscall_args[1] & MAX_PROCESSES_MSK;
    struct proc* process = &process_table[upid];
    if (process->state == UNALLOCATED) {
        current_process->return_value = -1;
        return;
    }
    process->state = DEAD;
    current_process->return_value = 0;
}

//int wait(pid_t upid)
void sys_wait()
{
    if (PROC_ALIVE(syscall_args[1])) {
        current_process->state = BLOCKED;
        current_process->waitpid_state.target_pid = syscall_args[1];
        current_process->syscall_state = SYSCALL_STATE_BEGIN;
    } else {
        current_process->return_value = -1;
    }
}

void sys_wait_update(struct proc* process)
{
    struct proc* target_proc = &process_table[process->waitpid_state.target_pid];

    if (target_proc->state == UNALLOCATED) {
        process->state = READY;
        process->return_value = target_proc->return_value;
        proc_enqueue(process);
        process->syscall_state = SYSCALL_STATE_NIL;
    }
}

void sys_mount()
{

}

void sys_mount_update(struct proc* process)
{

}

void sys_umount()
{

}

void sys_umount_update(struct proc* process)
{

}

//int sysctl(int cmd, void* buff, int count)
void sys_sysctl()
{
    int cmd = syscall_args[1];
    void* buff = (void*) syscall_args[2];
    int count = syscall_args[3];
    if (cmd == 0) {
        int count = 0;
        for (int i = 0; i < MAX_PROCESSES; i++) {
            struct proc* curr = &process_table[i];
            if (PROC_ALIVE(curr->pid)) {
                struct procinfo* pbuff = &((struct procinfo*) buff)[count++];
                pbuff->state = curr->state;
                pbuff->upid = curr->pid;
                pbuff->program_base = curr->program_base;
                pbuff->program_size = curr->program_size;
            }
        }
        current_process->return_value = count;
    } else if (cmd == 1) {
        char* ubuff = buff;
        strlcpy(ubuff, (char*) uname, count - 1);
        current_process->return_value = 0;
    }
}

void syscall_update()
{
    struct proc* current = &process_table[0];
    struct proc* end = process_table + 4;
    
    while(current < end) {

        if (current->syscall_state != SYSCALL_STATE_NIL && current->state != UNALLOCATED) {
            syscall_update_table[current->syscall_operation](current);
        }

        current++;
    }
}

void process_syscall()
{
    
    uint32_t syscallno = syscall_args[0];
    
    if (syscallno < SYSCALL_COUNT) {
        current_process->syscall_operation = syscallno;
        syscall_setup_table[syscallno]();
    } else {
        //bad syscall
    }
    //schedule new thread
    proc_update();

}

