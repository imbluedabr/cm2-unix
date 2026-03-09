#include <arch/mcxa153/MCXA153.h>
#include <arch/mcxa153/PERI_LPUART.h>
#include <arch/mcxa153/interrupt.h>

#include <uapi/majors.h>
#include <lib/kprint.h>
#include <kernel/device.h>
#include <kernel/devtbl.h>
#include <kernel/proc.h>
#include <fs/fs.h>
#include <fs/devfs.h>

#include <drivers/usart.h>

void uart0_init(int baudrate)
{
    MRCC0->MRCC_LPUART0_CLKSEL = MRCC_MRCC_LPUART0_CLKSEL_MUX(2);
    MRCC0->MRCC_LPUART0_CLKDIV = 0;

    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_LPUART0(1);
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_PORT0(1);

    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_CC0_LPUART0(1);
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_CC0_PORT0(1);

    PORT0->PCR[2] = PORT_PCR_LK(1) | PORT_PCR_MUX(2) | PORT_PCR_IBE(1);
    PORT0->PCR[3] = PORT_PCR_LK(1) | PORT_PCR_MUX(2);
    
    LPUART0->BAUD = LPUART_BAUD_OSR(0b01111) | LPUART_BAUD_SBR(CLK_FRO_48MHZ / (baudrate * 16));
    LPUART0->CTRL |= LPUART_CTRL_TE(1) | LPUART_CTRL_RE(1);
}

void uart0_putc(char c)
{
    while(!(LPUART0->STAT & LPUART_STAT_TDRE_MASK));
    
    LPUART0->DATA = c;
}

void uart0_puts(const char* str)
{
    while(*str != '\0') {
        uart0_putc(*str++);
    }
}

void gpio_output_init(void)
{
    MRCC0->MRCC_GLB_CC1 |= MRCC_MRCC_GLB_CC1_GPIO3(1);
    MRCC0->MRCC_GLB_CC1 |= MRCC_MRCC_GLB_CC1_PORT3(1);
    MRCC0->MRCC_GLB_RST1 |= MRCC_MRCC_GLB_RST1_GPIO3(1);
    MRCC0->MRCC_GLB_RST1 |= MRCC_MRCC_GLB_RST1_PORT3(1);

    PORT3->PCR[13] = 0x00008000;
    
    GPIO3->PDOR |= (1<<13);

    GPIO3->PDDR |= (1<<13);
}


void init_cpu()
{
    SCB->CPACR |= ((3UL << 0*2) | (3UL << 1*2));    /* set CP0, CP1 Full Access in Secure mode (enable PowerQuad) */
//#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
//  SCB_NS->CPACR |= ((3UL << 0*2) | (3UL << 1*2));    /* set CP0, CP1 Full Access in Normal mode (enable PowerQuad) */
//#endif /* (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U) */

    SCB->NSACR |= ((3UL << 0) | (3UL << 10));   /* enable CP0, CP1, CP10, CP11 Non-secure Access */

/*
#if !defined(__ZEPHYR__)
#if defined(__MCUXPRESSO)
    extern void(*const g_pfnVectors[]) (void);
    SCB->VTOR = (uint32_t) &g_pfnVectors;
#else
    extern void *__Vectors;
    SCB->VTOR = (uint32_t) &__Vectors;
#endif
#endif
*/

    /* Enable the LPCAC */
    SYSCON->LPCAC_CTRL |= SYSCON_LPCAC_CTRL_LPCAC_MEM_REQ_MASK;
    SYSCON->LPCAC_CTRL &= ~SYSCON_LPCAC_CTRL_DIS_LPCAC_MASK;

    /* Enable flash RWX when FLASH_ACL in IFR0 is invalid */
    if ((*((volatile const uint32_t *)(0x1000000)) == 0xFFFFFFFFU) ||
        ((*((volatile const uint32_t *)(0x1000000)) == 0x59630000U) &&
         (*((volatile const uint32_t *)(0x1000040)) == 0xFFFFFFFFU) &&
         (*((volatile const uint32_t *)(0x1000044)) == 0xFFFFFFFFU)))
    {
        /* Enable MBC register written with GLIKEY index15 */
        GLIKEY0->CTRL_0 = 0x00060000U;
        GLIKEY0->CTRL_0 = 0x0002000FU;
        GLIKEY0->CTRL_0 = 0x0001000FU;
        GLIKEY0->CTRL_1 = 0x00290000U;
        GLIKEY0->CTRL_0 = 0x0002000FU;
        GLIKEY0->CTRL_1 = 0x00280000U;
        GLIKEY0->CTRL_0 = 0x0000000FU;

        /* Enable RWX for GLBAC0 */
        MBC0->MBC_INDEX[0].MBC_MEMN_GLBAC[0] = 0x7700U;

        /* Use GLBAC0 for all flash block */
        for (uint8_t i = 0; i < 2U; i++)
        {
            MBC0->MBC_INDEX[0].MBC_DOM0_MEM0_BLK_CFG_W[i] = 0x00000000U;
        }

        /* Disable MBC register written */
        GLIKEY0->CTRL_0 = 0x0002000FU;
    }

    /* Route the PMC bandgap buffer signal to the ADC */
    SPC0->CORELDO_CFG |= (1U << 24U);

    /* Enables flash speculation */
    SYSCON->NVM_CTRL &= ~(SYSCON_NVM_CTRL_DIS_MBECC_ERR_DATA_MASK | SYSCON_NVM_CTRL_DIS_MBECC_ERR_INST_MASK);
    SYSCON->NVM_CTRL &= ~SYSCON_NVM_CTRL_DIS_FLASH_SPEC_MASK;


}

void main()
{
    init_cpu();
    //interrupt_init();
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
    
    device_node_t table[] = {
        {
            .major = USART_MAJOR,
            .arg = &(struct usart_desc) {
                .base = LPUART0,
                .device_id = 0,
                .baudrate = 9600
            }
        }
    };

    devtbl_init(table, 1);

    console = device_lookup(MKDEV(USART_MAJOR, 0));
    
    uart0_init(9600);
    while(1) {

        uart0_puts("Hello world!\r\n");
        //kputs("Hello world!\r\n");
        for (volatile int i = 0; i < 0x200000; i++) {
            device_update();
        };

    }
}





