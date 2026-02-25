#include <fs/fs.h>
#include <lib/stdlib.h>

struct inode inode_table[INODE_TABLE_SIZE];

uint8_t free_list[INODE_TABLE_SIZE];
uint8_t free_list_size;


struct fd fd_table[MAX_FD];

//this is just a private type
struct fs_tuple {
    struct super_ops* fs;
    char name[FS_NAME_LEN];
};

//this maps names to fileystem types
struct fs_tuple filesystem_registry[MAX_FILESYSTEM_COUNT];



void fs_init()
{
    free_list_size = INODE_TABLE_SIZE;

    for (int i = 0; i < INODE_TABLE_SIZE; i++) {
        free_list[i] = i;
    }
    for (int i = 0; i < MAX_FD; i++) {
        fd_table[i].file = NULL;
    }
}

struct inode* create_inode(const char* name)
{
    uint8_t tmp = free_list_size;
    if (tmp == 0) {
        return NULL;
    }
    free_list_size = --tmp;
    struct inode* new = &inode_table[tmp];
    new->refcount = 0;
    strlcpy(new->name, (char*) name, FS_INAME_LEN);
    return new;
}

void free_inode(struct inode* i)
{
    if (i->fs == NULL) {
        return;
    }
    if (i->refcount != 0) {
        i->refcount--;
    } else {
        i->fs = NULL;
        free_list[free_list_size++] = i - inode_table;
    }
}

struct inode* get_inode_ref(struct inode* i)
{
    if (i->refcount == 255) {
        return NULL;
    }
    i->refcount++;
    return i;
}


//TODO: improve inode lookup speed
int8_t lookup_dir(fs_lookup_t* state)
{
    if (FS_IS_A_FILE(state->dir->mode)) {
        return -1; //its not a dir
    }

    if (state->dir->mode == FS_MODE_MOUNT) {
        state->fs = state->dir->fs;
    }

    //check if the inode is already openend
    if (state->req == NULL) {
                
        struct inode* current = &inode_table[0];
        struct inode* end = inode_table + INODE_TABLE_SIZE;
        
        while (current < end) {

            //check if the file is in the right dir
            if (current->dir == state->dir->file) {
                if (strncmp(current->name, state->fname, FS_INAME_LEN) == 0) {//then compare the names
                    state->dir = current;
                    return 1;
                }
            }

            current++;
        }
    }

    int8_t stat = state->fs->sops->lookup(state);
    return stat;
}

struct inode* lookup_ino(ino_t ino)
{
    struct inode* current = &inode_table[0];
    struct inode* end = inode_table + INODE_TABLE_SIZE;
    
    while (current < end) {

        //check if the inode numbers are equal
        if (current->file == ino) {
            return current;
        }

        current++;
    }
    return NULL;
}

void register_filesystem(const char* name, struct super_ops* fs)
{
    for (int i = 0; i < MAX_FILESYSTEM_COUNT; i++) {
        if (filesystem_registry[i].fs == NULL) {
            filesystem_registry[i].fs = fs;
            strncpy(filesystem_registry[i].name, (char*) name, FS_NAME_LEN);
            break;
        }
    }
}

struct super_ops* lookup_filesystem(const char* name)
{
    for (int i = 0; i < MAX_FILESYSTEM_COUNT; i++) {
        if (strncmp(filesystem_registry[i].name, name, FS_NAME_LEN) == 0) {
            return filesystem_registry[i].fs;
        }
    }
    return NULL;
}

//maybe there is a faster way of allocating these?
int fd_alloc()
{
    for (int i = 0; i < MAX_FD; i++) {
        if (fd_table[i].file == NULL) {
            return i;
        }
    }
    return -1;
}



