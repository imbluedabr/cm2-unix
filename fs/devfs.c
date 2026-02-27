#include <fs/devfs.h>
#include <lib/stdlib.h>
#include <lib/kprint.h>

const struct super_ops devfs_sops = {
    .lookup = &devfs_lookup,
    .mount = &devfs_mount
};

const struct file_ops devfs_fops = {
    .read = &devfs_read,
    .write = &devfs_write,
    .readdir = &devfs_readdir
};


struct devfs_superblock devfs = {
    .base.fops = (struct file_ops*) &devfs_fops,
    .base.sops = (struct super_ops*) &devfs_sops
};

int8_t devfs_lookup(fs_lookup_t* state)
{
    if (*state->fname == '\0') {
        return 1;

    }
    for (int i = 0; i < DEVFS_MAX_FILES; i++) {
        struct devfs_file* file = &devfs.files[i];
        if (file->devno == 255) {
            continue;
        }
        if (strncmp(file->name, state->fname, FS_INAME_LEN) == 0) {
            struct inode* new = create_inode(file->name);
            new->dir = DEVFS_CREATE_INO(DEVFS_MAX_FILES);
            new->file = DEVFS_CREATE_INO(i);
            new->fs = &devfs.base;
            new->mode = FS_MODE_DEV;
            new->devfs.devno = file->devno;
            state->dir = new;
            return 1;
        }
    }
    return -1;
}


int devfs_mount(struct inode* mountpoint, dev_t devno, const char* args)
{
    mountpoint->file = DEVFS_CREATE_INO(DEVFS_MAX_FILES);
    mountpoint->fs = &devfs.base;
    for (int i = 0; i < DEVFS_MAX_FILES; i++) {
        devfs.files[i].devno = 255;
    }
    return 0;
}

int8_t devfs_read(fs_read_t* state)
{
    if (state->req == NULL) {
        state->req = device_newreq(state->buffer, state->count, state->descriptor->offset, DEVICE_OP_RD);
        struct device* dev = device_lookup(state->descriptor->file->devfs.devno);
        device_queue_action(dev, state->req);
        return 0;
    }

    if (state->req->state == DEVICE_STATE_FINISHED) {
        state->bytes_read = state->req->count;
        state->descriptor->offset += state->bytes_read;
        device_free_req(state->req);
        return 1;
    }

    return 0;
}

int8_t devfs_write(fs_write_t* state)
{
    if (state->req == NULL) {
        state->req = device_newreq(state->buffer, state->count, state->descriptor->offset, DEVICE_OP_WR);
        struct device* dev = device_lookup(state->descriptor->file->devfs.devno);
        device_queue_action(dev, state->req);
        return 0;
    }

    if (state->req->state == DEVICE_STATE_FINISHED) {
        state->bytes_written = state->req->count;
        state->descriptor->offset += state->bytes_written;
        device_free_req(state->req);
        return 1;
    }

    return 0;
}

int8_t devfs_readdir(fs_read_t* state) {
    struct inode* dir = state->descriptor->file;
    if (FS_IS_A_FILE(dir->mode)) {
        return -1;
    }

    uint16_t dir_i = state->bytes_read >> 16;
    uint16_t buff_i = state->bytes_read & 0xFFFF;

    struct devfs_file* file_entry = &devfs.files[dir_i];
    struct stat* dirent = &((struct stat*) state->buffer)[buff_i];
    
    if (file_entry->devno != 255) {
        ++buff_i;

        dirent->dev = file_entry->devno;
        dirent->time = 0;
        dirent->mode = FS_MODE_DEV;
        dirent->size = 0; //should use ioctl to find size of device
        dirent->d_ino = DEVFS_CREATE_INO(dir_i-1);
        strlcpy(dirent->name, (char*) file_entry->name, FS_INAME_LEN);
    }
    state->bytes_read = ((++dir_i) << 16) | buff_i;

    if (buff_i == state->count || dir_i == DEVFS_MAX_FILES) {
        state->bytes_read &= 0xFFFF;
        return 1;
    } 
    return 0;

}

int devfs_create_handle(const char* name, dev_t devno)
{
    for (int i = 0; i < DEVFS_MAX_FILES; i++) {
        struct devfs_file* handle = &devfs.files[i];
        if (handle->devno == 255) {
            handle->devno = devno;
            strncpy(handle->name, (char*) name, FS_INAME_LEN);
            return 0;
        }
    }
    return -1;
}



