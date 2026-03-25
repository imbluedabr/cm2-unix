#include <lib/kprint.h>
#include <lib/stdlib.h>
#include <lib/alloc.h>

#include <kernel/device.h>
#include <kernel/devtbl.h>
#include <kernel/proc.h>
#include <kernel/exec.h>
#include <kernel/syscall.h>
#include <kernel/panic.h>
#include <kernel/globals.h>
#include <kernel/settings.h>

#include <fs/romfs.h>
#include <fs/devfs.h>
#include <fs/fatfs.h>

#include <uapi/tty.h>
#include <uapi/majors.h>
#include <drivers/usart.h>
#include <drivers/tty.h>
#include <drivers/cm2disk.h>
#include <drivers/tilegpu.h>

void main() {
    
    device_node_t devconfig[] = {
        {
            .major = USART_MAJOR,
            .arg = &(struct usart_desc) {
                .base = (void*) 0xFFF1,
                .device_id = 0
            }
        },
        {
            .major = TTY_MAJOR,
            .arg = &(struct tty_desc) {
                .read_dev = MKDEV(USART_MAJOR, 0),
                .write_dev = MKDEV(USART_MAJOR, 0)
            }
        },
        {
            .major = GEN_DISK_MAJOR,
            .arg = (void*) 0xFFC3
        },
        {
            .major = TILEGPU_MAJOR,
            .arg = &(struct tilegpu_hw_interface) {
                .controls = TILEGPU_CONTROLS,
                .fx_imm = TILEGPU_FX_IMM,
                .fx_opcode = TILEGPU_FX_OPCODE,
                .tile_id = TILEGPU_ADDR,
                .y = TILEGPU_Y,
                .x = TILEGPU_X
            }
        }
    };
    
    //initialize functions
    heap_init((uint8_t*) 0x6000);
    device_init();
    proc_init();
    fs_init();
    devfs_init();

#ifdef USART_DRIVER
    usart_init();
#endif
#ifdef TTY_DRIVER
    tty_init();
#endif
#ifdef CM2_BLOCK_DEV
    gen_disk_init();
#endif
#ifdef CM2_TILING_GPU
    tilegpu_init();
#endif
    
    devtbl_init(devconfig, 3);
    console = device_lookup(MKDEV(USART_MAJOR, 0));
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
    kputs("Mounted filesystems!\nStarting init...\n");
    
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


