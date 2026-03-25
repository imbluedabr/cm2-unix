
/*
    .text
    .thumb
    .extern current_process

    .thumb_func
    .align 2
    .globl   save_user_context
    .weak    save_user_context
    .type    save_user_context, %function
save_user_context:
    ldmia sp,{r0, r1, r2, r3}
    ldr r4, =current_process
    stmia r4,{r0, r1, r2, r3}


*/




