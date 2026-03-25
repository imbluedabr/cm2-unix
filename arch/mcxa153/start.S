
    .syntax unified
    .arch armv8-m.main
/* early boot vector table */
    .section .boot_vtable, "a"
    .align 2
    .globl __Vectors
    .extern systick_handler
    .extern lpuart_handler
__Vectors:
    .long   __stack_top                                      /* Top of Stack */
    .long   reset_handler                                   /* Reset Handler */
    .long   0                                     /* NMI Handler*/
    .long   0                               /* Hard Fault Handler*/
    .long   0                               /* MPU Fault Handler*/
    .long   0                                /* Bus Fault Handler*/
    .long   0                              /* Usage Fault Handler*/
    .long   0                             /* Secure Fault Handler*/
    .long   0                                               /* Reserved*/
    .long   0                                               /* Reserved*/
    .long   0                                               /* Reserved*/
    .long   0                                     /* SVCall Handler*/
    .long   0                                /* Debug Monitor Handler*/
    .long   0                                               /* Reserved*/
    .long   0                                  /* PendSV Handler*/
    .long   systick_handler                                 /* SysTick Handler*/
    
    .size   __Vectors, . - __Vectors







    .text
    .thumb
    .extern main

    .thumb_func
    .align 2
    .globl   reset_handler
    .weak    reset_handler
    .type    reset_handler, %function
reset_handler:
    cpsid   i 
    .equ    VTOR, 0xE000ED08
    ldr     r0, =VTOR
    ldr     r1, =__Vectors
    str     r1, [r0]
    ldr     r2, [r1]
    msr     msp, r2             /* why the fuck are we setting the msp again?? its automaticly set by the hardware?? */
    ldr     r0, =__stack_limit
    msr     msplim, r0


    ldr    r1, =__DATA_ROM
    ldr    r2, =__data_start
    ldr    r3, =__data_end

.LC0:
    cmp     r2, r3
    ittt    lt
    ldrlt   r0, [r1], #4
    strlt   r0, [r2], #4
    blt    .LC0

    ldr r1, =__bss_start
    ldr r2, =__bss_end

    movs    r0, 0
.LC2:
    cmp     r1, r2
    itt    lt
    strlt   r0, [r1], #4
    blt    .LC2



/* stack initialization */
    ldr     r1, =__stack_limit
    ldr     r2, =__stack_top
.LC4:
    cmp     r1, r2
    itt     lt
    strlt   r0, [r1], #4
    blt     .LC4


    cpsie   i               /* Unmask interrupts */
    ldr r0, =main
    bx r0                   /* jump to the main function */
    .pool
    .size reset_handler, . - reset_handler







