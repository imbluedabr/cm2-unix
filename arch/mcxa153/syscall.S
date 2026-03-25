/* supervisor call handler */
/*
    .text
    .thumb
    .extern syscall_args
    .extern process_syscall
    .thumb_func
    .align 2
    .globl   SVC_handler
    .weak    SVC_handler
    .type    SVC_handler, %function
SVC_handler:
    ldmia sp,{r0, r1, r2, r3}
    ldr r4, =syscall_args
    stmia r4,{r0, r1, r2, r3}
    PUSH {lr}
    bl process_syscall
    POP {lr}
    
    bx lr
*/





