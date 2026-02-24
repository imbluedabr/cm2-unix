#include <kernel/exec.h>
#include <kernel/syscall.h>
#include <lib/alloc.h>
#include <lib/kprint.h>
#include <lib/stdlib.h>
#include <stddef.h>

#include <arch/riscv/cm2exef.h>

int proc_exec(exe_t* exec_state, const char* path, const char** argv, int* fileno_vec)
{
    if (path == NULL) {
        return -1;
    }

    walk_path_init(&exec_state->walker, path);
    exec_state->descriptor.file = NULL;
    exec_state->argv = argv;
    
    //translate the fd numbers into file descriptor pointers
    int i = 0;
    
    memset(exec_state->file_buff, 0, EXEC_MAX_PASSED_FD*4);
    if (fileno_vec != NULL) {
        while(fileno_vec[i] > -1 && i < EXEC_MAX_PASSED_FD) {
            struct fd* file = proc_get_fd(fileno_vec[i]);
            if (file == NULL) {
                return -1;
            }
            exec_state->file_buff[i++] = file;
        }
    }
    return 0;
}

int proc_exec_update(exe_t* exec_state)
{
    struct fd* f = &exec_state->descriptor;
    fs_read_t* rstate = &exec_state->fs;

    if (f->file) {
        int8_t stat = rstate->fs->fops->read(rstate);
        
        if (stat < 0) {
            return -1;
        } else if (stat == 1) {
            uint32_t program_base = (uint32_t) rstate->buffer;
            struct cm2exef_header* hdr = rstate->buffer;
            //kprintf("base: %x\n", program_base + sizeof(struct cm2exef_header));
            
            struct proc* new = proc_create(
                    program_base + sizeof(struct cm2exef_header),
                    program_base + hdr->initial_sp,
                    exec_state->argv,
                    exec_state->file_buff
                    );
            new->program_base = program_base;
            new->program_size = hdr->initial_sp;
            return new->pid;
        }
        return 0;
    }

	int8_t rt = walk_path(&exec_state->walker);
	if (rt < 0) {
		return -1;
	} else if (rt == 1) {
        struct inode* file = exec_state->walker.fs_state.dir;
        if (!FS_IS_A_FILE(file->mode)) {
            return -1;
        }
        void *program = malloc(file->size + 2048);
		if (!program) {
			return -1; //out of memory condition
		}
        
        f->file = file;
        f->flags = 0;
        f->offset = 0; //start at the begining of the file
        //set the read state
        set_read_state(rstate, f, program, file->size);
	}
    return 0;
}

