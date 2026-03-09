#pragma once
#include <kernel/device.h>

int mcxa_lpuart_ioctl(struct device* dev, int cmd, void* arg);
int mcxa_lpuart_readb(struct device* dev);
int mcxa_lpuart_writeb(struct device* dev, uint8_t val);
void mcxa_lpuart_update(struct device* dev);





