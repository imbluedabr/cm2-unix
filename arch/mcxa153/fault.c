#include <arch/mcxa153/fault.h>
#include <kernel/panic.h>
#include <arch/mcxa153/MCXA153.h>


void NMI_handler()
{
    GPIO3->PTOR = 1 << 13;
}

void hardfault_handler()
{
    GPIO3->PTOR = 1 << 13;
}

void MPUfault_handler()
{
    GPIO3->PTOR = 1 << 13;
}

void busfault_handler()
{
    GPIO3->PTOR = 1 << 13;
}

void usagefault_handler()
{
    GPIO3->PTOR = 1 << 13;
}

void securefault_handler()
{
    //GPIO3->PTOR ^= 1 << 13;
}

void pendsv_handler()
{

}



