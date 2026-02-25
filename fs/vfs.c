#include <fs/vfs.h>
#include <kernel/proc.h>
#include <stddef.h>
#include <lib/stdlib.h>
#include <lib/kprint.h>
#include <kernel/panic.h>

struct inode rootfs;

void walk_path_init(path_walk_t* state, struct inode* rel_base, const char* path)
{
    memset(state->path_cpy, 0, FS_PATH_LEN);
    int path_index = 0;
    if (*path == '/') { //absolute path
        path_index++;
        state->fs_state.dir = &rootfs;
        state->fs_state.fs = rootfs.fs; //we start the search at the root
    } else if (rel_base != NULL) {
        state->fs_state.dir = rel_base;
        state->fs_state.fs = rel_base->fs;
    } else {
        panic("relative path without base");
    }
    
    strlcpy(state->path_cpy + path_index, (char*) path + path_index, FS_PATH_LEN);
    
    state->fs_state.fname = state->path_cpy + path_index;
    state->path_index = path_index;
    state->path = path;
    state->fs_state.req = NULL;
}


int8_t walk_path(path_walk_t* state)
{
    
    if (state->path[state->path_index] == '\0') {
        int8_t stat = lookup_dir(&state->fs_state); 
        return stat;
    }
    
    int8_t stat = 1;
    
    if (state->path[state->path_index] == '/') {
        state->path_cpy[state->path_index] = '\0';
        stat = lookup_dir(&state->fs_state);
        if (stat < 0) {
            return stat; //directory not found
        } else if (stat == 1) {
            state->fs_state.fname = state->path_cpy + state->path_index + 1;
        }
    }
    
    if (stat == 1) {
        state->path_index++;
    }

    return 0; //continue
}


void construct_cwd(char* buff, const char* path, uint8_t full)
{
    int i = 0;
    
    if (path[0] == '/') {
        path++;
        i++;
    } else {
        i = strnlen(buff, FS_PATH_LEN);
        if (i > 1) {
            buff[i++] = '/';
            if (i > FS_PATH_LEN) {
                return;
            }
        }
    }
    const char* word = path;
    while(*path != '\0') {
        if (*path++ == '/') {
            int size = path - word;
            if ((size + i) > FS_PATH_LEN) {
                break;
            }
            if (size != 1) {
                memcpy(&buff[i], (void*) word, size);
                i += size;
            }
            word = path;
        }
    }
    
    int size = path - word;
    if (i + size > FS_PATH_LEN) {
        return;
    }
    strlcpy(&buff[i], (void*) word, size);
}


int mount_root(const char* fs_name, dev_t devno)
{
    struct super_ops* fs = lookup_filesystem(fs_name);
    if (fs == NULL) {
        return -1;
    }
        
    return fs->mount(&rootfs, devno, NULL);
}

int mount_devfs(const char* fs_name)
{
    fs_lookup_t currstate = {
        .req = NULL,
        .fs = rootfs.fs,
        .dir = &rootfs,
        .fname = "dev"
    };

    int8_t rt = 0;
    while (rt == 0) {
        rt = rootfs.fs->sops->lookup(&currstate);
        device_update();
    }
    if (rt == -1) {
        return -1;
    }
    currstate.dir->mode = FS_MODE_MOUNT;
    return lookup_filesystem(fs_name)->mount(currstate.dir, 255, NULL);
}


int8_t vfs_mount_init(vfs_mount_t* state, const char* path, const char* fs_name, dev_t devno)
{
    state->devno = devno;
    state->fs = lookup_filesystem(fs_name);
    if (state->fs == NULL) {
        return -1;
    }
    walk_path_init(&state->path, current_process->cwd_inode, path);
    return 0;
}

int8_t vfs_mount_update(vfs_mount_t* state)
{
    int8_t stat = walk_path(&state->path);
    if (stat < 0) {
        return stat;
    }
    if (stat == 1) {
        struct inode* i = state->path.fs_state.dir;
        if (state->fs->mount(i, state->devno, NULL) < 0) {
            return -1;
        };
        i->mode = FS_MODE_MOUNT;
        return 1;
    }
    return 0;
}


void vfs_close(int fileno)
{
    struct fd* descriptor = proc_get_fd(fileno);
    if (descriptor == NULL) {
        return;
    }
    free_inode(descriptor->file);
    descriptor->file = NULL;
    current_process->open_files[fileno] = PROC_FILE_NIL;
}


