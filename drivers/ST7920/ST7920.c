#include <drivers/ST7920.h>
#include <arch/mcxa153/MCXA153.h>
#include <uapi/majors.h>
#include <uapi/ST7920.h>
#include <lib/kprint.h>
#include <stddef.h>
#include <stdbool.h>

static struct st7920_device display;

//st7920 operations
const struct device_ops st7920_ops = {
    .readb = &st7920_readb,
    .writeb = &st7920_writeb,
    .ioctl = &st7920_ioctl,
    .update = &st7920_update
};

void st7920_init() {
    struct device_driver* st7920_driver = &driver_registry[ST7920_MAJOR];
    st7920_driver->create = &st7920_create;
    st7920_driver->destroy = &st7920_destroy;
    st7920_driver->lookup = &st7920_lookup;
    st7920_driver->update = &st7920_global_update;
    st7920_driver->name = "st7920 driver";
}

#define RS_SHIFT 8
#define EN_MASK (1 << 9)

static inline void tiny_delay(int loops)
{
    for (volatile int i = 0; i < loops; i++) {
        __NOP();
    }
}

static inline void write_port(struct st7920_device* disp, uint8_t rs, uint8_t val)
{
    GPIO1->PCOR = 0x3FF; //clear the parralel bus
    GPIO1->PSOR = (rs << RS_SHIFT) | val;
    tiny_delay(1);
    GPIO1->PSOR = EN_MASK;
    tiny_delay(1);
    GPIO1->PCOR = EN_MASK;
    tiny_delay(1);
    GPIO1->PCOR = 0x3FF;
}

static int st7920_push_cmd(struct st7920_device* disp, uint8_t rs, uint8_t val)
{
    uint8_t tmp = (disp->cmd_head + 1) & (ST7920_CMD_BUFF_SIZE - 1);
    if (tmp == disp->cmd_tail) {
        return -1;
    }
    disp->cmd_head = tmp;
    disp->cmd_buff[tmp] = (struct st9720_cmd) { .data = val, .rs = rs};
    return 0;
}


static int cmd_function_set(struct st7920_device* disp, bool RE)
{
    return st7920_push_cmd(disp, 0, (0b11 << 4) | (RE << 2));
}

static int cmd_display_control(struct st7920_device* disp, bool display, bool cursor, bool blink)
{
    return st7920_push_cmd(disp, 0, (1 << 3) | (display << 2) | (cursor << 1) | blink);
}

static int cmd_entry_mode_set(struct st7920_device* disp, bool ID, bool S)
{
    return st7920_push_cmd(disp, 0, (1 << 2) | (ID << 1) | S);
}

static int cmd_display_clear(struct st7920_device* disp)
{
    return st7920_push_cmd(disp, 0, 1);
}

static int cmd_set_ddram(struct st7920_device* disp, uint8_t val)
{
    return st7920_push_cmd(disp, 0, 0x80 | (val & 0x7F));
}

void st7920_reset(struct st7920_device* disp)
{
    MRCC0->MRCC_GLB_CC1 |= MRCC_MRCC_GLB_CC1_GPIO1(1);
    MRCC0->MRCC_GLB_CC0 |= MRCC_MRCC_GLB_CC0_PORT1(1);
    MRCC0->MRCC_GLB_RST1 |= MRCC_MRCC_GLB_RST1_GPIO1(1);
    MRCC0->MRCC_GLB_RST0 |= MRCC_MRCC_GLB_RST0_PORT1(1);
    
    for (int i = 0; i < 10; i++) {
        PORT1->PCR[i] = 0x00008000;
        GPIO1->PCOR = (1<<i);
        
        GPIO1->PDDR |= (1<<i);
    }
    
    write_port(disp, 0, 0b00110000);
    tiny_delay(1000);
    write_port(disp, 0, 0b00110000);
    tiny_delay(1000);
    write_port(disp, 0, 0b00001111);
    tiny_delay(1000);
    write_port(disp, 0, 0b00000110);
    tiny_delay(1000);
    write_port(disp, 0, 0b00000001);
    tiny_delay(5000);
}

struct device* st7920_create(int8_t* minor, const void* args)
{
    if (display.base.ops == NULL) {
        display.base.ops = (struct device_ops*) &st7920_ops;
        display.base.count = 0; //init queue
        display.base.head = 0;
        display.base.tail = 0;
        display.cursor_pos = 0;
        display.mode = 0;
        display.cmd_head = 0;
        display.cmd_tail = 0;
        *minor = 0;
        st7920_reset(&display);
        return &display.base;
    }

    *minor = -1; //TODO: add error code for this
    return NULL;
}

int st7920_destroy(uint8_t minor)
{
    display.base.ops = NULL;
    return 0;
}

struct device* st7920_lookup(uint8_t minor)
{
    if ((display.base.ops == NULL) || (minor != 0)) {
        return NULL;
    }
    return &display.base;
}

void st7920_global_update() {
    if (display.base.ops != NULL) {
        st7920_update(&display.base);
    }
}

int st7920_ioctl(struct device* dev, int cmd, void* arg) {
    struct st7920_device* disp = (struct st7920_device*) dev;
    
    int return_code = 0;

    if (disp->base.ops == NULL) {
        return 0;
    }
    switch(cmd) {
        case IOCTL_RESET:
            st7920_reset(disp);
            break;
        default:
            return_code = -1;
    }

    return return_code;
}

int st7920_readb(struct device* dev)
{
    return -1;
}

static const uint8_t DDRAM_addr[4] = {
    0x00,
    0x10,
    0x8,
    0x18
};

int st7920_writeb(struct device* dev, uint8_t val)
{
    struct st7920_device* disp = (struct st7920_device*) dev;
    if (disp->mode & (1 << 0)) {
        if (cmd_set_ddram(disp, DDRAM_addr[(disp->cursor_pos >> 4)]) == -1) { return -1; };
        disp->mode &= ~(1 << 0);
    }

    if (disp->cursor_pos == 4*16) {
        if (cmd_display_clear(disp) == -1) { return -1; };
        disp->cursor_pos = 0;
    }
    if (val == '\n') {
        disp->cursor_pos &= 0xF0;
        disp->cursor_pos += 16;
    } else if (val == '\b') {
        disp->cursor_pos--;
    } else {
        if (st7920_push_cmd(disp, 1, val) == -1) { return -1; };
        disp->cursor_pos++;
    }
    if ((disp->cursor_pos & 0xF) == 0) {
        disp->mode |= (1 << 0);
        return -1;
    }
    return 0;
}

void st7920_update(struct device* dev)
{
    struct st7920_device* disp = (struct st7920_device*) dev;
    struct device_request* current_req = disp->current_req;
    if (disp->cmd_head != disp->cmd_tail) {
        uint8_t tmp = (disp->cmd_tail + 1) & (ST7920_CMD_BUFF_SIZE - 1);
        disp->cmd_tail = tmp;
        struct st9720_cmd* cmd = &disp->cmd_buff[tmp];
        write_port(disp, cmd->rs, cmd->data);
    }
    

    if (current_req == NULL) {
        current_req = device_queue_pop(dev);
        if (current_req == NULL)
            return;

        disp->current_req = current_req;
        disp->bytes_copied = 0;
    }
    
    if (current_req->operation == DEVICE_OP_WR) {
        
    } else {
        
    }

}








