#define DEBUG 1

#include <init.h>
#include <asm/io.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

typedef struct {
    uint32_t pad[18];
    uint32_t gates[10];
} s5l87xx_clkcon;

#define S5L87XX_CLKCON ((volatile s5l87xx_clkcon *)0x3C500000)

typedef struct {
    uint8_t gate;
    uint8_t bit;
} s5l87xx_clkgate_index; 

typedef struct {
    char *id;
    s5l87xx_clkgate_index clkgate1;
    // Some of the clockgate mappings have two clockgates. If so, this field will be non-zero.
    s5l87xx_clkgate_index clkgate2;
} s5l87xx_clkgate_mapping; 

static const s5l87xx_clkgate_mapping *s5l87xx_clkgate_mappings[] = {
    &(s5l87xx_clkgate_mapping) {
        .id = "timer0", .clkgate1 = { 1, 5 }, .clkgate2 = { 9, 0 },
    },
    &(s5l87xx_clkgate_mapping) {
        .id = "timer1", .clkgate1 = { 1, 23 }, .clkgate2 = { 9, 1 },
    },
    &(s5l87xx_clkgate_mapping) {
        .id = "timer2", .clkgate1 = { 1, 24 }, .clkgate2 = { 9, 2 },
    },
    &(s5l87xx_clkgate_mapping) {
        .id = "timer3", .clkgate1 = { 1, 25 }, .clkgate2 = { 9, 3 },
    },
    &(s5l87xx_clkgate_mapping) {
        .id = "timer4", .clkgate1 = { 1, 26 }, .clkgate2 = { 9, 4 },
    },
    &(s5l87xx_clkgate_mapping) {
        .id = "timer5", .clkgate1 = { 1, 27 }, .clkgate2 = { 9, 5 },
    },
    &(s5l87xx_clkgate_mapping) {
        .id = "timer6", .clkgate1 = { 1, 28 }, .clkgate2 = { 9, 6 },
    },
    &(s5l87xx_clkgate_mapping) {
        .id = "timer7", .clkgate1 = { 4, 5 }, .clkgate2 = { 9, 22 },
    },
    &(s5l87xx_clkgate_mapping) {
        .id = "timer8", .clkgate1 = { 4, 6 }, .clkgate2 = { 9, 23 },
    },
    &(s5l87xx_clkgate_mapping) {
        .id = "uart0", .clkgate1 = { 1, 9 }, .clkgate2 = { 9, 7 },
    },
    &(s5l87xx_clkgate_mapping) {
        .id = "usb-otg", .clkgate1 = { 0, 2 },
    },
    &(s5l87xx_clkgate_mapping) {
        .id = "usb2-phy", .clkgate1 = { 1, 3 },
    },
    NULL,
};

static void s5l87xx_enable_clkgate_bit(uint8_t gate, uint8_t bit) {
    uint32_t mask = ~(((uint32_t) 1) << bit);
    S5L87XX_CLKCON->gates[gate] &= mask;
}

static void s5l87xx_enable_clkgate(const char *id) {
    s5l87xx_clkgate_mapping const **mapping = s5l87xx_clkgate_mappings;
    while (mapping != NULL) {
        const s5l87xx_clkgate_mapping *m = *mapping;
        if (strcmp(m->id, id) != 0) {
            mapping++;
            continue;
        }

        debug("s5l87xx: ungating %s\n", id);
        s5l87xx_enable_clkgate_bit(m->clkgate1.gate, m->clkgate1.bit);
        if ((m->clkgate2.gate != 0) && (m->clkgate2.bit != 0)) {
            s5l87xx_enable_clkgate_bit(m->clkgate2.gate, m->clkgate2.bit);
        }
        return;
    }
    panic("s5l87xx_enable_clkgate: unknown id %s", id);
}

struct s5l87xx_uart {
    uint32_t ulcon;    // 0x00
    uint32_t ucon;     // 0x04
    uint32_t ufcon;    // 0x08
    uint32_t pad1;     // 0x0c
    uint32_t utrstat;  // 0x10
    uint32_t pad2[3];  // 0x14
    uint32_t utxh;     // 0x20
    uint32_t pad3;     // 0x24
    uint32_t ubrdiv;   // 0x28
    uint32_t pad4[2];  // 0x2c
    uint32_t ubrconrx; // 0x34
    uint32_t ubrcontx; // 0x38
};

// These are all undocumented. The following is gathered from
// reverse-engineering work of the original iPod firmware.
//
// Reference: https://en.wikipedia.org/wiki/Korean_profanity

struct s5l87xx_timer {
    uint32_t con;     // 0x000
    uint32_t cmd;     // 0x004
    uint32_t data0;   // 0x008
    uint32_t data1;   // 0x00c
    uint32_t pre;     // 0x010
    uint32_t cnt;     // 0x014
};

struct s5l87xx_otgphy {
    uint32_t pwr;
    uint32_t con;
    uint32_t rstcon;
    uint32_t unk[4];
    uint32_t unkcon;
};

struct s5l87xx_buscon {
    uint32_t unk[3];
    uint32_t remap;
};

struct s5l87xx_lcdcon {
    uint32_t con;    // 0x00
    uint32_t cmd;    // 0x04
    uint32_t unk1;   // 0x08
    uint32_t unk2;   // 0x0C
    uint32_t ack;    // 0x10
    uint32_t read;   // 0x14
    uint32_t unk3;   // 0x18
    uint32_t status; // 0x1C
    uint32_t unk[8]; // 0x20
    uint32_t write;  // 0x40
};

#define S5L87XX_LCDCON ((volatile struct s5l87xx_lcdcon *)0x38300000)

static void s5l87xx_lcdcon_read_byte(uint8_t *out) {
    udelay(100);
    writel(0, &S5L87XX_LCDCON->ack);

    uint32_t status;
    do {
        status = readl(&S5L87XX_LCDCON->status);
    } while((status & 1) == 0);

    udelay(100);

    uint32_t data = readl(&S5L87XX_LCDCON->read);
    if (out != NULL) {
        *out = (data >> 1);
    }
}

static void s5l87xx_lcdcon_wait_ready() {
    debug("%s: start...\n", __func__);
    uint32_t status;
    do {
        status = readl(&S5L87XX_LCDCON->status);
    } while((status & (1<<4)) != 0);
    debug("%s: done.\n", __func__);
}

static void s5l87xx_lcdcon_transact_read(uint32_t cmd, uint32_t len, uint8_t *out) {
    writel(0x1000c20, &S5L87XX_LCDCON->con);
    s5l87xx_lcdcon_wait_ready();
    writel(cmd, &S5L87XX_LCDCON->cmd);

    // Discard first byte???
    s5l87xx_lcdcon_read_byte(out);

    for (uint32_t i = 0; i < len; i++) {
        s5l87xx_lcdcon_read_byte(out);
        debug("%s: out: %02x\n", __func__, *out);
        out++;
    }
}

enum s5l87xx_lcd_type {
    S5L87XX_LCD_TYPE_UNSUPPORTED = 0,
    S5L87XX_LCD_TYPE_48C4 = 1,
    S5L87XX_LCD_TYPE_38B3 = 2,
    S5L87XX_LCD_TYPE_38F7 = 4
};

static enum s5l87xx_lcd_type s5l87xx_lcdcon_get_type(void) {
    uint8_t id[3] = {0};
    s5l87xx_lcdcon_transact_read(4, 3, id);
    if (id[0] == 0x48 && id[1] == 0xc4) {
        return S5L87XX_LCD_TYPE_48C4;
    }
    if (id[0] == 0x38) {
        if (id[1] == 0xb3) {
            return S5L87XX_LCD_TYPE_38B3;
        }
        if (id[1] == 0xf7) {
            return S5L87XX_LCD_TYPE_38F7;
        }
    }
    return S5L87XX_LCD_TYPE_UNSUPPORTED;
}

void s5l87xx_lcd_init(void) {
    enum s5l87xx_lcd_type type = s5l87xx_lcdcon_get_type();
    const char* types = "UNKNOWN";
    switch (type) {
    case S5L87XX_LCD_TYPE_48C4:
        types = "48c4";
    case S5L87XX_LCD_TYPE_38B3:
        types = "38b3";
    case S5L87XX_LCD_TYPE_38F7:
        types = "38f7";
    }
    debug("%s: detected LCD type %s (%d)\n", __func__, types, type);
}

enum s5l87xx_buscon_remap {
    S5L87XX_BUSCON_REMAP_ENABLE = 1,
    S5L87XX_BUSCON_REMAP_SRAM = 2,
};

static void s5l87xx_buscon_remap_sdram(void) {
    debug("s5l87xx_buscon_remap_sdram\n");
    volatile struct s5l87xx_buscon *buscon = (struct s5l87xx_buscon *)0x3E000000;
    buscon->remap = S5L87XX_BUSCON_REMAP_ENABLE;
}

static void s5l87xx_otgphy_off(void) {
    debug("s5l87xx_otgphy: turning off\n");
    volatile struct s5l87xx_otgphy *otgphy = (struct s5l87xx_otgphy *)0x3c400000;
    otgphy->pwr = 0xff;
    mdelay(10);
    otgphy->rstcon = 0xff;
    mdelay(10);
    otgphy->unkcon = 4;
}

static void s5l87xx_otgphy_on(void) {
    // TODO(q3k): lmao
    s5l87xx_lcd_init();

    debug("s5l87xx_otgphy: turning on\n");
    s5l87xx_enable_clkgate("usb-otg");
    s5l87xx_enable_clkgate("usb2-phy");
    mdelay(10);

    volatile struct s5l87xx_otgphy *otgphy = (struct s5l87xx_otgphy *)0x3c400000;
    otgphy->pwr = 0;
    mdelay(10);
    otgphy->rstcon = 1;
    mdelay(10);
    otgphy->rstcon = 0;
    mdelay(10);
    otgphy->unkcon = 6;
    otgphy->con = 1;
    mdelay(400);
}

void otg_phy_init(void *unused) {
    s5l87xx_otgphy_on();
}

void otg_phy_off(void *unused) {
    s5l87xx_otgphy_off();
}

enum s5l87xx_timer_id {
    //  Timers A, B, C, D: 16-bit
    S5L87XX_TIMER_A = 0,
    S5L87XX_TIMER_B = 1,
    S5L87XX_TIMER_C = 2,
    S5L87XX_TIMER_D = 3,
    // Timer E: 64-bit (unimplemented, different registers from others)
    S5L87XX_TIMER_E = 4,
    // Timers F, G, H, I: 32-bit
    S5L87XX_TIMER_F = 5,
    S5L87XX_TIMER_G = 6,
    S5L87XX_TIMER_H = 7,
    S5L87XX_TIMER_I = 8,
};

enum s5l87xx_timer_cmd {
    S5L87XX_TIMER_CMD_STOP = 0,
    S5L87XX_TIMER_CMD_START = 1,
    S5L87XX_TIMER_CMD_CLR = 2,
};

static struct s5l87xx_timer *s5l87xx_timer_registers(enum s5l87xx_timer_id id) {
    switch (id) {
    case S5L87XX_TIMER_A:
        return (struct s5l87xx_timer *)0x3c700000;
    case S5L87XX_TIMER_B:
        return (struct s5l87xx_timer *)0x3c700020;
    case S5L87XX_TIMER_C:
        return (struct s5l87xx_timer *)0x3c700040;
    case S5L87XX_TIMER_D:
        return (struct s5l87xx_timer *)0x3c700060;
    case S5L87XX_TIMER_E:
        return (struct s5l87xx_timer *)0x3c700080;
    case S5L87XX_TIMER_F:
        return (struct s5l87xx_timer *)0x3c7000a0;
    case S5L87XX_TIMER_G:
        return (struct s5l87xx_timer *)0x3c7000c0;
    case S5L87XX_TIMER_H:
        return (struct s5l87xx_timer *)0x3c7000e0;
    case S5L87XX_TIMER_I:
        return (struct s5l87xx_timer *)0x3c700100;
    default:
        panic("requested invalid timer id %d", id);
    }
}

static const char* s5l87xx_timer_clockgate(enum s5l87xx_timer_id id) {
    switch (id) {
    case S5L87XX_TIMER_A:
        return "timer0";
    case S5L87XX_TIMER_B:
        return "timer1";
    case S5L87XX_TIMER_C:
        return "timer2";
    case S5L87XX_TIMER_D:
        return "timer3";
    case S5L87XX_TIMER_E:
        return "timer4";
    case S5L87XX_TIMER_F:
        return "timer5";
    case S5L87XX_TIMER_G:
        return "timer6";
    case S5L87XX_TIMER_H:
        return "timer7";
    case S5L87XX_TIMER_I:
        return "timer8";
    default:
        panic("requested invalid timer id %d", id);
    }
}

static void s5l87xx_timer_configure_interval(enum s5l87xx_timer_id id) {
    debug("s5l87xx_timer: configuring %d in interval mode\n", id);
    s5l87xx_enable_clkgate(s5l87xx_timer_clockgate(id));

    volatile struct s5l87xx_timer *timer = s5l87xx_timer_registers(id);

    timer->cmd = S5L87XX_TIMER_CMD_STOP;
    timer->con = 0x40;
    timer->pre = 0xb;
    timer->data0 = 0xffffffff;
    timer->cmd = S5L87XX_TIMER_CMD_CLR;
}

static void s5l87xx_timer_start(enum s5l87xx_timer_id id) {
    debug("s5l87xx_timer: starting %d\n", id);
    volatile struct s5l87xx_timer *timer = s5l87xx_timer_registers(id);
    timer->cmd = S5L87XX_TIMER_CMD_START;
}

static void s5l87xx_timer_stop(enum s5l87xx_timer_id id) {
    debug("s5l87xx_timer: stopping %d\n", id);
    volatile struct s5l87xx_timer *timer = s5l87xx_timer_registers(id);
    timer->cmd = S5L87XX_TIMER_CMD_STOP;
}

static uint32_t s5l87xx_timer_read(enum s5l87xx_timer_id id) {
    volatile struct s5l87xx_timer *timer = s5l87xx_timer_registers(id);
    return timer->cnt;
}

int timer_init(void)
{
    s5l87xx_timer_configure_interval(S5L87XX_TIMER_F);
    s5l87xx_timer_start(S5L87XX_TIMER_F);

    return 0;
}

unsigned long timer_read_counter(void)
{
    return s5l87xx_timer_read(S5L87XX_TIMER_F);
}

// TODO(q3k): move board early init to board
int board_early_init_f(void)
{
    debug("board_early_init_f\n");
    // HACKHACKHACK add a pmctrl to linux
    // needed for timer c0..???
    s5l87xx_enable_clkgate("timer3");
    // HACKHACKHACK

    // Disable all VIC interrupts.
    // TODO(q3k): disable VIC elsewhere
    static volatile uint32_t *vic0_enclr = (uint32_t *)0x38e00014;
    static volatile uint32_t *vic1_enclr = (uint32_t *)0x38e01014;
    *vic0_enclr = 0xffffffff;
    *vic1_enclr = 0xffffffff;

    s5l87xx_enable_clkgate("usb-otg");
    s5l87xx_enable_clkgate("usb2-phy");

    // Disable USB suspend. TODO(q3k): move this to DWC2?
    volatile uint32_t *pcgcctl = (uint32_t *)0x38400e00;
    *pcgcctl = 0;
    return 0;
}


#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
    s5l87xx_enable_clkgate("uart0");

    // Enable GPIO pins on N5G.
    static volatile uint32_t *gpio = (uint32_t *)0x3cf00000;
    *gpio &= 0xff00ffff;
    *gpio |= 0x00220000;
}

#endif
