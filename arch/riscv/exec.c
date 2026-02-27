#include <kernel/exec.h>
#include <kernel/syscall.h>
#include <kernel/panic.h>
#include <lib/alloc.h>
#include <lib/kprint.h>
#include <lib/stdlib.h>
#include <stddef.h>

#include <arch/riscv/cm2exef.h>

int proc_exec(exe_t* exec_state, struct inode* cwd, const char* path, const char** argv, int* fileno_vec)
{
    if (path == NULL) {
        return -1;
    }

    walk_path_init(&exec_state->walker, cwd, path);
    exec_state->descriptor.file = NULL;
    exec_state->argv = argv;
    exec_state->state = WALK_PATH;
    
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

int proc_exec_update(exe_t* exec_state, struct proc* process)
{
    int8_t rt;
    struct fd* f = &exec_state->descriptor;
    fs_read_t* rstate = &exec_state->fs;

    switch(exec_state->state) {
        case WALK_PATH:
            rt = walk_path(&exec_state->walker);
            if (rt < 0) {
                return -1;
            } else if (rt == 1) {
                struct inode* file = get_inode_ref(exec_state->walker.fs_state.dir);
                if (!FS_IS_A_FILE(file->mode)) {
                    rt = -1;
                    goto exit;
                }
                                
                f->file = file;
                f->flags = 0;
                f->offset = 0; //start at the begining of the file

                void* hdr = malloc(sizeof(struct cm2exef_header));
                if (hdr == NULL) {
                    rt = -1;
                    goto exit;
                }
                set_read_state(rstate, f, hdr, sizeof(struct cm2exef_header));
                exec_state->state = LOAD_HEADER;
	        }
            break;
        case LOAD_HEADER:
            rt = rstate->fs->fops->read(rstate);

            if (rt < 0) {
                goto exit;
            } else if (rt == 1) {
                struct cm2exef_header* hdr = rstate->buffer;
                int size = hdr->program_break;
                kprintf("size: %x\n", size);
                //free the memory used by the header
                free(rstate->buffer);

                void* program = malloc(size);
                if (program == NULL) {
                    rt = -1;
                    goto exit;
                }
                f->offset = 0;
                set_read_state(rstate, f, program, size);
                exec_state->state = LOAD_BODY;
            }
            break;
        case LOAD_BODY:
            rt = rstate->fs->fops->read(rstate);

            if (rt < 0) {
                goto exit;
            } else if (rt == 1) {
                uint32_t program_base = (uint32_t) rstate->buffer;
                struct cm2exef_header* hdr = rstate->buffer;

                struct proc* new = proc_create(
                        program_base + sizeof(struct cm2exef_header),
                        program_base + hdr->program_break,
                        exec_state->argv,
                        exec_state->file_buff
                        );

                new->cwd_inode = get_inode_ref(process->cwd_inode);
                strlcpy(new->cwd_path, process->cwd_path, FS_PATH_LEN);
                
                new->program_base = program_base;
                new->program_size = hdr->program_break;
                rt = new->pid;
                goto exit;
            }
    }
    return 0;
    
    exit:
    free_inode(f->file);
    return rt;
}

