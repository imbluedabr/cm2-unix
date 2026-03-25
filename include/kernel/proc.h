#pragma once
#include <stdint.h>
#include <uapi/proc.h>
#include <kernel/settings.h>
#include <kernel/device.h>
#include <fs/vfs.h>

typedef struct {
    uint8_t target_pid;
} waitpid_t;

typedef struct {
    path_walk_t walker;
} vfs_open_t;

typedef struct {
    fs_read_t fs;
} vfs_read_t;

typedef struct {
    fs_write_t fs;
} vfs_write_t;

#define EXEC_MAX_PASSED_FD 4
typedef struct {
    struct fd descriptor;
    const char** argv;
    struct fd* file_buff[EXEC_MAX_PASSED_FD];
	path_walk_t walker;
	fs_read_t fs;
    enum : uint8_t {
        WALK_PATH,
        LOAD_HEADER,
        LOAD_BODY
    } state;
} exe_t;

typedef struct {
    path_walk_t walker;
} chdir_t;

#define SYSCALL_STATE_NIL 255
#define SYSCALL_STATE_BEGIN 0

#define PROC_MAXFILES 16
#define PROC_FILE_NIL 255


struct proc {
    
#if defined (ARCH_MCXA153)
    uint32_t saved_regs[13];
    uint32_t user_sp;
    uint32_t link_register;
    uint32_t return_address;
#elif defined (ARCH_TAURUS)
    uint32_t return_address;
    uint32_t user_sp;
    uint32_t saved_regs[12];
#endif

    uint32_t return_value;
    enum proc_state state;

    //this is for syscalls to remember how far they have progressed
    uint8_t syscall_state;
    uint8_t syscall_operation;
    pid_t pid;
    
    //per process file descript table
    uint8_t open_files[PROC_MAXFILES];
    
    uint32_t program_base;
    uint32_t program_size;
    
    //current working directory
    struct inode* cwd_inode;
    char cwd_path[FS_PATH_LEN + 1];

    //this is the state that multi tick syscalls need
    union {
        waitpid_t waitpid_state;
        vfs_open_t open_state;
        vfs_read_t read_state;
        vfs_write_t write_state;
        exe_t exec_state;
        chdir_t chdir_state;
    };
    
};

#define MAX_PROCESSES 4
#define MAX_PROCESSES_MSK (MAX_PROCESSES - 1)
extern struct proc process_table[MAX_PROCESSES];
extern uint8_t free_processes[MAX_PROCESSES];
extern uint8_t free_processes_count;


//0 - 1 is blocked or ready, and 2 - 3 is dead or unallocated so if we check bit 0b10 we know if the process is alive or dead
#define PROC_ALIVE(PID) ((process_table[PID & MAX_PROCESSES_MSK].state & 0b10) == 0)


extern struct proc* current_process;

extern uint32_t kernel_sp;

int proc_enqueue(struct proc* process);
struct proc* proc_dequeue();
void proc_init();
struct proc* proc_create(uint32_t entry_point, uint32_t stack_pointer, const char** argv, struct fd** file_vec);
void proc_delete(struct proc* process);
void proc_resume(struct proc* process, int return_value);
struct fd* proc_get_fd(int fd);
uint8_t proc_alloc_fd(struct proc* process);
void proc_update();



