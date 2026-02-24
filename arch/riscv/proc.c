#include <kernel/proc.h>
#include <kernel/device.h>
#include <lib/stdlib.h>
#include <lib/kprint.h>
#include <lib/alloc.h>
#include <kernel/syscall.h>
#include <kernel/panic.h>
#include <stddef.h>

struct proc process_table[MAX_PROCESSES];

uint8_t free_processes[MAX_PROCESSES];
uint8_t free_processes_count;

struct proc* ready_queue[MAX_PROCESSES];
uint8_t ready_queue_head;
uint8_t ready_queue_tail;
uint8_t ready_queue_count;

struct proc* current_process;

uint32_t kernel_sp;

//add this process to the queue of processes ready to execute
int proc_enqueue(struct proc* process)
{
    if (ready_queue_count == MAX_PROCESSES) return -1;
    
    ready_queue[ready_queue_tail] = process;
    ready_queue_tail = (ready_queue_tail + 1) & MAX_PROCESSES_MSK;
    ready_queue_count++;
    
    return 0;
}

struct proc* proc_dequeue()
{
    if (ready_queue_count == 0) return NULL;
    
    struct proc* process = ready_queue[ready_queue_head];
    ready_queue_head = (ready_queue_head + 1) & MAX_PROCESSES_MSK;
    ready_queue_count--;
    
    return process;
}



void proc_init()
{
    ready_queue_head = 0;
    ready_queue_tail = 0;
    ready_queue_count = 0;
    current_process = NULL;

    free_processes_count = MAX_PROCESSES;


    for (int i = 0; i < MAX_PROCESSES; i++) {
        free_processes[i] = i;
        process_table[i].state = UNALLOCATED;
        memset(&process_table[i].open_files, PROC_FILE_NIL, PROC_MAXFILES);
    }
}

struct proc* proc_create(uint32_t entry_point, uint32_t stack_pointer, const char **argv, struct fd** file_vec) {
    
    uint8_t tmp = free_processes_count;
    if (tmp == 0) return NULL;
    uint8_t index = free_processes[--tmp];
    struct proc* new_process = &process_table[index];
    free_processes_count = tmp;

    new_process->pid = index;
    new_process->return_address = entry_point;
    new_process->user_sp = stack_pointer;
    new_process->state = READY;
    new_process->return_value = 0;
    new_process->syscall_state = SYSCALL_STATE_NIL;

    new_process->program_base = 0;
    new_process->program_size = 0;
    
    memset(new_process->open_files, PROC_FILE_NIL, PROC_MAXFILES);

    if (file_vec != NULL) {
        //pass the fd's to the new process
        int file_vec_count = 0;
        while(file_vec[file_vec_count] != NULL && file_vec_count < EXEC_MAX_PASSED_FD) {
            struct fd* file = file_vec[file_vec_count++];
            int new_fd_num = fd_alloc();
            struct fd* file_cpy = &fd_table[new_fd_num];
            file_cpy->file = file->file;
            file->file->refcount++;
            file_cpy->flags = file->flags;
            file_cpy->offset = file->offset;
            int new_fileno = proc_alloc_fd(new_process);
            new_process->open_files[new_fileno] = new_fd_num;
        }
    }

    if (argv != NULL) {
        //TODO: actualy implement correct passing of argv and argc, not whatever this shit is
        new_process->return_value = (uint32_t) argv;
    }

    proc_enqueue(new_process);
    return new_process;
}

void proc_delete(struct proc* process) {
    if (process->state == DEAD) {
        process->state = UNALLOCATED;
        free_processes[free_processes_count++] = process->pid;
        
        for (int i = 0; i < PROC_MAXFILES; i++) {
            vfs_close(process->open_files[i]);
        }

        free((void*) process->program_base);
    }
}

void proc_resume(struct proc* process, int return_value)
{
    process->state = READY;
    process->return_value = return_value;
    proc_enqueue(process);
    process->syscall_state = SYSCALL_STATE_NIL;
}


struct fd* proc_get_fd(int fd) {
    if (fd < 0 || fd > MAX_FD) {
        return NULL;
    }
    uint8_t fdnum = current_process->open_files[fd];
    if (fdnum > PROC_MAXFILES) {
        return NULL;
    }
    struct fd* descriptor = &fd_table[fdnum];

    return descriptor;
}

uint8_t proc_alloc_fd(struct proc* process) {
    for (int i = 0; i < PROC_MAXFILES; i++) {
        if (process->open_files[i] == PROC_FILE_NIL) {
            return i;
        }
    }
    return PROC_FILE_NIL;
}




void proc_update()
{
    struct proc* process = current_process;
    
    //if the current process is ready we queue it up for execution
    if (process->state == READY && process != NULL) {
        proc_enqueue(process);
    }
    if (process->state == DEAD && process != NULL) {
        proc_delete(process);
    }

    if (free_processes_count == MAX_PROCESSES) { //out of processes
        panic("out of processes");
    }

    //get a new process from the top of the queue
    struct proc* new_process = NULL;
    while(new_process == NULL) {
        device_update(); //do a kernel 'tick' basicly call all update functions
        syscall_update();
        new_process = proc_dequeue();
    }
    //kprintf("pid: %x\n", new_process->pid);

    current_process = new_process;
    
}

