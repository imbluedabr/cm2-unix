#include <kernel/device.h>

struct usart_desc {
    void* base;
    int device_id;
};


struct usart_device {
    struct device base;
    struct device_request* current_req;
    void* usart_base;
    uint16_t bytes_copied;
    uint16_t mode;
};

void usart_init();
struct device* usart_create(int8_t* minor, const void* args);
int usart_destroy(uint8_t minor);
struct device* usart_lookup(uint8_t minor);
void usart_global_update();

