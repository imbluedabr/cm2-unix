#include <fs/vfs.h>
#include <kernel/proc.h>
#include <stddef.h>
#include <lib/stdlib.h>
#include <lib/kprint.h>

struct inode rootfs;

void walk_path_init(path_walk_t* state, const char* path)
{
    memset(state->path_cpy, 0, FS_PATH_LEN);
    if (*path == '/') { //absolute path
        path++;
    } else {
        //relative paths not suported yet
    }
    
    strlcpy(state->path_cpy, (char*) path, FS_PATH_LEN - 1);
    
    state->fs_state.fname = state->path_cpy;
    state->path_index = 0;
    state->path = path;
    state->fs_state.dir = &rootfs;
    state->fs_state.fs = rootfs.fs; //we start the search at the root
    state->fs_state.req = NULL;
}


int8_t walk_path(path_walk_t* state)
{
    
    if (state->path[state->path_index] == '\0') {
        
        int8_t stat = lookup_dir(&state->fs_state);
        if (stat == 1) {

        }
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


int8_t mount_init(vfs_mount_t* state, const char* path, const char* fs_name, dev_t devno)
{
    state->devno = devno;
    state->fs = lookup_filesystem(fs_name);
    if (state->fs == NULL) {
        return -1;
    }
    walk_path_init(&state->path, path);
    return 0;
}

int8_t mount_update(vfs_mount_t* state)
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


