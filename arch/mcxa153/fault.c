#include <arch/mcxa153/fault.h>
#include <kernel/panic.h>



void NMI_handler()
{
    panic("NMI Interrupt!");
}

void hardfault_handler()
{
    panic("HardFault exception!");
}

void MPUfault_handler()
{
    panic("MPUFault exception");
}

void busfault_handler()
{
    panic("BusFault exception!");
}

void usagefault_handler()
{
    
}

void securefault_handler()
{

}

void pendsv_handler()
{

}

void systick_handler()
{

}



