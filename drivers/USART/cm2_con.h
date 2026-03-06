#include <kernel/device.h>

#define CM2CON_ESC_STATE (1 << 0) //'\e' detected
#define CM2CON_CSI_STATE (1 << 1) //"\[" csi control sequence

int cm2con_ioctl(struct device* dev, int cmd, void* arg);
int cm2con_readb(struct device* dev);
int cm2con_writeb(struct device* dev, uint8_t val);
void cm2con_update(struct device* dev);


