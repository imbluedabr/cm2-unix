#include <kernel/device.h>

struct usart_desc {
    void* base;
    int device_id;
    int baudrate;
    int irq;
};


struct usart_device {
    struct device base;
    struct device_request* current_req;
    void* usart_base;
    uint16_t bytes_copied;
    uint16_t mode;
    char rx_buff[16];
    char tx_buff[16];
    uint8_t rx_head;
    uint8_t rx_tail;
    uint8_t tx_head;
    uint8_t tx_tail;
};

//this is the table that stores the tty instances
#define MAX_USART_COUNT 2
#define MAX_USART_MSK (MAX_USART_COUNT-1)
extern struct usart_device usart_table[MAX_USART_COUNT];


void usart_init();
struct device* usart_create(int8_t* minor, const void* args);
int usart_destroy(uint8_t minor);
struct device* usart_lookup(uint8_t minor);
void usart_global_update();

