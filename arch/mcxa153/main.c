#include <stddef.h>
#include <arch/mcxa153/MCXA153.h>
#include <arch/mcxa153/PERI_LPUART.h>
#include <arch/mcxa153/interrupt.h>

#include <uapi/majors.h>
#include <uapi/tty.h>
#include <uapi/ST7920.h>
#include <lib/kprint.h>
#include <lib/stdlib.h>
#include <kernel/device.h>
#include <kernel/devtbl.h>
#include <kernel/proc.h>
#include <kernel/uname.h>
#include <kernel/settings.h>
#include <fs/fs.h>
#include <fs/devfs.h>

#include <drivers/tty.h>
#include <drivers/usart.h>
#include <drivers/ST7920.h>

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

static volatile uint32_t ticks = 0;

void systick_handler()
{
    device_update();
    ticks++;
}

void main()
{
    init_cpu();
    interrupt_init();
    device_init();
    proc_init();
    fs_init();
    devfs_init();

    gpio_output_init();
   
#ifdef USART_DRIVER
    usart_init();
#endif
#ifdef TTY_DRIVER
    tty_init();
#endif
#ifdef ST7920_DRIVER
    st7920_init();
#endif

    SysTick_Config(48000);
    
    register_interrupt(SysTick_IRQn, NULL, systick_handler);
    
    device_node_t device_table[] = {
        {
            .major = USART_MAJOR,
            .arg = &(struct usart_desc) {
                .base = LPUART0,
                .device_id = 1,
                .baudrate = 9600,
                .irq = LPUART0_IRQn
            }
        },
        {
            .major = ST7920_MAJOR
        }
    };

    devtbl_init(device_table, 2);

    __enable_irq();
    console = device_lookup(MKDEV(ST7920_MAJOR, 0));
    
    //struct device* display = device_lookup(MKDEV(ST7920_MAJOR, 0));
    kputs(uname.sysname);
    kputc(' ');
    kputs(uname.release);
    kputc(' ');
    kputs(uname.version);
    kputc(' ');
    kputs(uname.machine);
    while(1) {
        if (ticks > 1000) {
            ticks = 0;
            GPIO3->PTOR = (1 << 13);
        }
    }
}





