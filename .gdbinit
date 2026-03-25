target remote localhost:3333
set mem inaccessible-by-default off
file main.elf
monitor reset halt
monitor set vector-catch all
alias m = monitor
