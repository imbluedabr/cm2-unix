
.section .text.start
.global _start

.extern main
.extern exit
.extern stdin
.extern stdout

#magic
.word 0xE85250D6
#abi
.byte 1
#isa
.byte 0
#abi major version
.byte 0
#abi minor version
.byte 0
#initial sp
.word _stack_top


_start:
#    la t0, stdout
#    sb x0, 0(t0)
#    la t0, stdin
#    sb x0, 0(t0)
    call main
    #a0 is the return value of main which is also the argument register for exit
    call exit



