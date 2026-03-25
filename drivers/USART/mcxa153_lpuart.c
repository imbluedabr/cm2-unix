#include "mcxa153_lpuart.h"
#include <stddef.h>
#include <arch/mcxa153/MCXA153.h>
#include <arch/mcxa153/PERI_LPUART.h>
#include <arch/mcxa153/interrupt.h>
#include <drivers/usart.h>


void mcxa153_lpuart_IRQHandler()
{
    struct usart_device* usart = (struct usart_device*) get_current_device();

    volatile LPUART_Type* lpuart = usart->usart_base;
    GPIO3->PTOR = 1 << 13;
    
    if (usart->tx_tail != usart->tx_head) {
        uint8_t tmp = (usart->tx_tail + 1) & 15;
        usart->tx_tail = tmp;
        lpuart->DATA = usart->tx_buff[tmp];
    } else {
        lpuart->CTRL &= ~LPUART_CTRL_TIE_MASK;
    }
    char c = lpuart->DATA;
    uint8_t tmp = (usart->rx_head + 1) & 15;
    if (usart->rx_tail != usart->rx_head) {
        usart->rx_buff[tmp] = c;
        usart->rx_head = tmp;
    }

    NVIC_ClearPendingIRQ(get_current_interrupt());
}

void mcxa153_lpuart_init(struct device* dev, struct usart_desc* desc)
{
    volatile LPUART_Type* lpuart = desc->base;
    MRCC0->MRCC_LPUART0_CLKSEL = MRCC_MRCC_LPUART0_CLKSEL_MUX(2);
    MRCC0->MRCC_LPUART0_CLKDIV = 0;

    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_LPUART0(1);
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_PORT0(1);

    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_CC0_LPUART0(1);
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_CC0_PORT0(1);

    PORT0->PCR[2] = PORT_PCR_LK(1) | PORT_PCR_MUX(2) | PORT_PCR_IBE(1);
    PORT0->PCR[3] = PORT_PCR_LK(1) | PORT_PCR_MUX(2);
    
    register_interrupt(desc->irq, dev, mcxa153_lpuart_IRQHandler);
    NVIC_SetPriority(desc->irq, 3);
    NVIC_ClearPendingIRQ(desc->irq);
    NVIC_EnableIRQ(desc->irq);

    lpuart->BAUD = LPUART_BAUD_OSR(0b01111) | LPUART_BAUD_SBR(CLK_FRO_48MHZ / (desc->baudrate * 16));
    lpuart->CTRL |= LPUART_CTRL_TE_MASK | LPUART_CTRL_RE_MASK | LPUART_CTRL_RIE_MASK;
}

int mcxa_lpuart_ioctl(struct device* dev, int cmd, void* arg)
{
    if (cmd == IOCTL_RESET) {
        struct usart_desc* desc = arg;
        mcxa153_lpuart_init(dev, desc);
    }
    return 0;
}

int mcxa_lpuart_readb(struct device* dev)
{
    struct usart_device* usart = (struct usart_device*) dev;
    
    if (usart->rx_tail == usart->rx_head) {
        return -1;
    }

    uint8_t tmp = (usart->rx_tail + 1) & 15;
    usart->rx_tail = tmp;

    return usart->rx_buff[tmp];
}

int mcxa_lpuart_writeb(struct device* dev, uint8_t val)
{
    struct usart_device* usart = (struct usart_device*) dev;
    
    uint8_t tmp = (usart->tx_head + 1) & 15;
    if (tmp == usart->tx_tail) {
        return -1;
    }
    usart->tx_head = tmp;
    usart->tx_buff[tmp] = val;
    volatile LPUART_Type* lpuart = usart->usart_base;
    lpuart->CTRL |= LPUART_CTRL_TIE_MASK;
    return 0;
}

static inline uint8_t mcxa_lpuart_write(
        struct usart_device* usart,
        struct device_request* current_req
        ) {
    uint16_t i = usart->bytes_copied;
    char c = ((char*) current_req->buffer)[i];

    if (mcxa_lpuart_writeb(&usart->base, c) == -1) {
        return 0;
    }

    usart->bytes_copied = ++i;

    if (i == current_req->count) {
        return 1;
    }
    return 0;
}

void mcxa_lpuart_update(struct device* dev)
{
    struct usart_device* usart = (struct usart_device*) dev;
    struct device_request* current_req = usart->current_req;
    /*
    volatile LPUART_Type* lpuart = usart->usart_base;
    if (lpuart->STAT & LPUART_STAT_TDRE_MASK) {
        if (usart->tx_head != usart->tx_tail) {
            uint8_t tmp = (usart->tx_tail + 1) & 15;
            usart->tx_tail = tmp;
            lpuart->DATA = usart->tx_buff[tmp];
        }
    }*/


    if (current_req == NULL) {
        current_req = device_queue_pop(dev);
        if (current_req == NULL)
            return;

        usart->current_req = current_req;
        usart->bytes_copied = 0;
    }
    
    uint8_t exit;
    if (current_req->operation == DEVICE_OP_WR) {
        exit = mcxa_lpuart_write(usart, current_req);
    } else {
        exit = 1;
    }

    if (exit) {
        current_req->state = DEVICE_STATE_FINISHED;
        current_req->count = usart->bytes_copied;
        usart->current_req = NULL;
    }
}


