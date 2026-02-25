#include <fs/fatfs.h>
#include <lib/stdlib.h>
#include <lib/kprint.h>
#include <stddef.h>


const struct super_ops fatfs_sops = {
    .lookup = &fatfs_lookup,
    .mount = &fatfs_mount
};

const struct file_ops fatfs_fops = {
    .read = &fatfs_read,
    .write = &fatfs_write,
    .readdir = &fatfs_readdir
};

struct fatfs_superblock fatfs = {
    .base.fops = (struct file_ops*) &fatfs_fops,
    .base.sops = (struct super_ops*) &fatfs_sops
};

int8_t fatfs_lookup(fs_lookup_t* state)
{
    uint8_t fat_index = state->dir->fatfs.fat_index;
    struct fatfs_superblock* fs = (struct fatfs_superblock*) state->fs;
    
    if (*state->fname == '\0') {
        return 1;
    }

    

    if (state->req != NULL && state->req->state == DEVICE_STATE_FINISHED) {
        device_free_req(state->req);
        state->req = NULL;
         
        for (uint8_t i = 0; i < FATFS_DIR_LEN; i++) {
            struct fatfs_file* entry = &fs->dir_cache[i];
            if (entry->fat_index == 0) {
                continue;
            }

            if (strncmp(entry->name, state->fname, FS_INAME_LEN) == 0) {
                struct inode* newi = create_inode(entry->name);
                newi->mode = entry->mode;
                newi->fs = &fs->base;
                newi->size = entry->size;
                newi->fatfs.fat_index = entry->fat_index;
                newi->dir = state->dir->file;
                newi->file = FATFS_CALC_INO(entry->fat_index);
                state->dir = newi;
                return 1;
            }
        }
        return -1;
    }

    if (state->req == NULL) {
        struct device_request* newreq = device_newreq(fs->dir_cache, FATFS_SECTOR_SIZE, CALC_SADRES(fat_index), DEVICE_OP_RD);
        device_queue_action(fs->dev, newreq);
        state->req = newreq;
    } 

    return 0;
}

int fatfs_mount(struct inode* mountpoint, dev_t devno, const char* args)
{
    mountpoint->fatfs.fat_index = 0; //set the root fat index
    mountpoint->mode = FS_MODE_MOUNT;
    mountpoint->file = FATFS_CALC_INO(0);
    mountpoint->fs = &fatfs.base;
    fatfs.devno = devno;
    fatfs.dev = device_lookup(devno);
    
    struct device_request* newreq = device_newreq(fatfs.fat_cache, FATFS_SECTOR_SIZE, 0, DEVICE_OP_RD);

    device_queue_action(fatfs.dev, newreq);

    while(newreq->state != DEVICE_STATE_FINISHED) {
        device_update();
    }
    device_free_req(newreq);
    return 0;
}

//TODO: make fatfs suport byte offsets instead of rounding everything to sectors
int8_t fatfs_read(fs_read_t* state)
{
    struct fatfs_superblock* fs = (struct fatfs_superblock*) state->fs;
    struct fd* desc = state->descriptor;

    uint8_t sectors_skipped = state->bytes_read >> 24;
    uint8_t current_sector = state->bytes_read >> 16;
    if (state->req == NULL) {
        current_sector = state->descriptor->file->fatfs.fat_index;
    }
    uint16_t bytes_read = state->bytes_read & 0xFFFF;

    //go to the start of the sectors we have to read
    if (sectors_skipped != (desc->offset >> 8)) {
        while(sectors_skipped < (desc->offset >> 8)) {
            current_sector = fs->fat_cache[current_sector];
            if (current_sector == FATFS_SECTOR_END) {
                return -1;
            }
            sectors_skipped++;
        }
    }

    if (state->req != NULL && state->req->state == DEVICE_STATE_FINISHED) {
        current_sector = fs->fat_cache[current_sector];
        device_free_req(state->req);

        if (current_sector == FATFS_SECTOR_END) {
            state->bytes_read = bytes_read;
            return 1;
        }
        state->req = NULL;
    }
    
    if (state->req == NULL) {
        uint16_t to_read = state->count - bytes_read;
        if (to_read > 0) {
            uint16_t tmp = (to_read > FATFS_SECTOR_SIZE) ? FATFS_SECTOR_SIZE : to_read;
            struct device_request* newreq = device_newreq(state->buffer + bytes_read, tmp, CALC_SADRES(current_sector), DEVICE_OP_RD);
            device_queue_action(fs->dev, newreq);
            bytes_read += tmp;
            state->req = newreq;

        } else {
            state->bytes_read = bytes_read;
            return 1;
        }
    }

    state->bytes_read = (sectors_skipped << 24) | (current_sector << 16) | bytes_read;

    return 0;
}

int8_t fatfs_write(fs_write_t* state)
{
    return -1;
}

int8_t fatfs_readdir(fs_read_t* state)
{
    struct fatfs_superblock* fs = (struct fatfs_superblock*) state->fs;
    
    if (state->req == NULL) {
        uint8_t index = state->descriptor->file->fatfs.fat_index;
        struct device_request* newreq = device_newreq(fs->dir_cache, FATFS_SECTOR_SIZE, CALC_SADRES(index), DEVICE_OP_RD);
        device_queue_action(fs->dev, newreq);
        state->req = newreq;
    } else if (state->req->state == DEVICE_STATE_FINISHED) {
        device_free_req(state->req);
        for (uint8_t i = 0; i < FATFS_DIR_LEN && state->bytes_read < state->count; i++) {
            struct fatfs_file* entry = &fs->dir_cache[i];
            if (entry->fat_index == 0) {
                continue;
            }
            
            struct stat* buff = &((struct stat*)state->buffer)[state->bytes_read++];
            
            buff->d_ino = FATFS_CALC_INO(entry->fat_index);
            buff->dev = fs->devno;
            buff->mode = entry->mode;
            buff->size = entry->size;
            buff->time = 0;
            strlcpy(buff->name, entry->name, FS_INAME_LEN);
        }
        return 1;
    }

    return 0;
}









