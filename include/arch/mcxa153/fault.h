#pragma once


void reset_handler();
void NMI_handler();
void hardfault_handler();
void MPUfault_handler();
void busfault_handler();
void usagefault_handler();
void securefault_handler();
void pendsv_handler();
void systick_handler();

