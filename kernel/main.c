#include <lib/kprint.h>
#include <lib/stdlib.h>

#include <kernel/device.h>
#include <drivers/tty.h>
#include <drivers/cm2disk.h>
#include <drivers/tilegpu.h>
#include <uapi/majors.h>
#include <kernel/proc.h>
#include <kernel/exec.h>
#include <kernel/syscall.h>
#include <kernel/panic.h>
#include <kernel/globals.h>
#include <kernel/settings.h>

#include <fs/romfs.h>
#include <fs/devfs.h>
#include <fs/fatfs.h>

void main() {
    dev_t tty0_devno, tty1_devno, gpu0_devno, disk0_devno;
    
    //initialize functions
    device_init();
    proc_init();
    fs_init();

#ifdef CM2_TTY_DRIVER
    tty_init();
    device_create(&tty0_devno, TTY_MAJOR, (void*) 0xFFF1);
    device_create(&tty1_devno, TTY_MAJOR, (void*) 0xFFBD);
#endif
#ifdef CM2_TILING_GPU
    tilegpu_init();
    device_create(&gpu0_devno, TILEGPU_MAJOR, &(struct tilegpu_hw_interface){
        .controls = TILEGPU_CONTROLS,
        .fx_imm = TILEGPU_FX_IMM,
        .fx_opcode = TILEGPU_FX_OPCODE,
        .tile_id = TILEGPU_ADDR,
        .y = TILEGPU_Y,
        .x = TILEGPU_X
    });
#endif
#ifdef CM2_BLOCK_DEV
    gen_disk_init();
    device_create(&disk0_devno, GEN_DISK_MAJOR, (void*) 0xFFC3);
#endif

    kputs(uname);
   
#ifdef FS_ROMFS
    register_filesystem("romfs", (struct super_ops*) &romfs_sops);
#endif
#ifdef FS_FATFS
    register_filesystem("fatfs", (struct super_ops*) &fatfs_sops);
#endif
    register_filesystem("devfs", (struct super_ops*) &devfs_sops);
    
    mount_root(ROOTFS_TYPE, ROOTFS_DEVNO);
    if (mount_devfs("devfs") < 0) {
        panic("devfs mount failed");
    }
    kputs("mounted filesystems!\n");
    
    devfs_create_handle("disk0", disk0_devno);
    devfs_create_handle("tty0", tty0_devno);
    devfs_create_handle("tty1", tty1_devno);

#ifdef CM2_TILING_GPU
    devfs_create_handle("gpu0", gpu0_devno);
#endif

    kputs("populated devfs!\nstarting init...\n");
    
    //exec the init process
    struct proc* boot = &process_table[0];
    boot->cwd_inode = &rootfs;
    strlcpy(boot->cwd_path, "/", 1);

    exe_t init;
    proc_exec(&init, &rootfs, INIT_PATH, NULL, NULL);
    int init_state = 0;
    while(init_state == 0) {
        init_state = proc_exec_update(&init, boot);
        device_update();
    }
    if (init_state < 0) {
        panic("failed to load init process");
    }
    
    //bootstrap the scheduler by getting the first process to run
    current_process = proc_dequeue();
    exit_kernel(); //jump into the current process
}


