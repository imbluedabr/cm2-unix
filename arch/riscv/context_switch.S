
.section .text.syscall

.extern current_process
.extern kernel_sp
.extern process_syscall
.extern syscall_args
.global syscall
.global exit_kernel

syscall:
    # load struct proc* current_process into t0
    la t0, current_process
    lw t0, 0(t0)

    sw ra, 0(t0) #current_process->return_adres = ra
    sw sp, 4(t0) #current_process->user_sp = sp
    
    sw s0, 8(t0) #current_process->saved_regs[0] = s0
    sw s1, 12(t0) #current_process->saved_regs[1] = s1
    sw s2, 16(t0) #current_process->saved_regs[2] = s2
    sw s3, 20(t0) #current_process->saved_regs[3] = s3
    sw s4, 24(t0) #current_process->saved_regs[4] = s4
    sw s5, 28(t0) #current_process->saved_regs[5] = s5
    sw s6, 32(t0) #current_process->saved_regs[6] = s6
    sw s7, 36(t0) #current_process->saved_regs[7] = s7
    sw s8, 40(t0) #current_process->saved_regs[8] = s8
    sw s9, 44(t0) #current_process->saved_regs[9] = s9
    sw s10, 48(t0) #current_process->saved_regs[10] = s10
    sw s11, 52(t0) #current_process->saved_regs[11] = s11
    
    # save the arguments passed
    la t0, syscall_args
    sw a0, 0(t0)
    sw a1, 4(t0)
    sw a2, 8(t0)
    sw a3, 12(t0)

    #load the kernel sp
    la t0, kernel_sp
    lw sp, 0(t0)
    
    #call the process_syscall function in arc/riscv/proc.c
    call process_syscall

exit_kernel:

    #store kernel stack
    la t0, kernel_sp
    sw sp, 0(t0)

    la t0, current_process
    lw t0, 0(t0)

    #load the return address
    lw ra, 0(t0)
    lw sp, 4(t0)

    lw s0, 8(t0)
    lw s1, 12(t0)
    lw s2, 16(t0)
    lw s3, 20(t0)
    lw s4, 24(t0)
    lw s5, 28(t0)
    lw s6, 32(t0)
    lw s7, 36(t0)
    lw s8, 40(t0)
    lw s9, 44(t0)
    lw s10, 48(t0)
    lw s11, 52(t0)

    #load return value
    lw a0, 56(t0)
    
    # return to the user
    ret


