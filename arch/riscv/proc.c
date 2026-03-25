#include <kernel/proc.h>
#include <kernel/syscall.h>
#include <kernel/panic.h>
#include <stddef.h>


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



