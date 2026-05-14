#include <stdint.h>
#include <init.h>
#include <asm/io.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

// Clockgate mapping definitions

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
        .id = "sha", .clkgate1 = { 0, 0 },
    },
    &(s5l87xx_clkgate_mapping) {
        .id = "lcd", .clkgate1 = { 0, 1 },
    },
    &(s5l87xx_clkgate_mapping) {
        .id = "usb-otg", .clkgate1 = { 0, 2 },
    },
    &(s5l87xx_clkgate_mapping) {
        .id = "smx", .clkgate1 = { 0, 3 },
    },
    &(s5l87xx_clkgate_mapping) {
        .id = "sm1", .clkgate1 = { 0, 4 },
    },
    &(s5l87xx_clkgate_mapping) {
        .id = "ata", .clkgate1 = { 0, 5 },
    },
    &(s5l87xx_clkgate_mapping) {
        .id = "nand", .clkgate1 = { 0, 8 },
    },
    &(s5l87xx_clkgate_mapping) {
        .id = "sdci", .clkgate1 = { 0, 9 },
    },
    &(s5l87xx_clkgate_mapping) {
        .id = "aes", .clkgate1 = { 0, 10 },
    },
    &(s5l87xx_clkgate_mapping) {
        .id = "nandecc", .clkgate1 = { 0, 12 },
    },
    &(s5l87xx_clkgate_mapping) {
        .id = "dmac0", .clkgate1 = { 0, 25 },
    },
    &(s5l87xx_clkgate_mapping) {
        .id = "dmac1", .clkgate1 = { 0, 26 },
    },
    &(s5l87xx_clkgate_mapping) {
        .id = "rom", .clkgate1 = { 0, 30 },
    },

    &(s5l87xx_clkgate_mapping) {
        .id = "rtc", .clkgate1 = { 1, 0 },
    },
    &(s5l87xx_clkgate_mapping) {
        .id = "cwheel", .clkgate1 = { 1, 1 },
    },
    &(s5l87xx_clkgate_mapping) {
        .id = "spi0", .clkgate1 = { 1, 2 },
    },
    &(s5l87xx_clkgate_mapping) {
        .id = "usb2-phy", .clkgate1 = { 1, 3 },
    },
    &(s5l87xx_clkgate_mapping) {
        .id = "i2c0", .clkgate1 = { 1, 4 },
    },
    &(s5l87xx_clkgate_mapping) {
        .id = "timer0", .clkgate1 = { 1, 5 }, .clkgate2 = { 9, 0 },
    },
    &(s5l87xx_clkgate_mapping) {
        .id = "uart0", .clkgate1 = { 1, 9 }, .clkgate2 = { 9, 7 },
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
    NULL,
};

static void s5l87xx_enable_clkgate_bit(uint8_t gate, uint8_t bit) {
    uint32_t mask = ~(((uint32_t) 1) << bit);
    S5L87XX_CLKCON->gates[gate] &= mask;
}

static void s5l87xx_enable_clkgate(const char *id) {
    s5l87xx_clkgate_mapping const **mapping = s5l87xx_clkgate_mappings;
    while (*mapping != NULL) {
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

// UART Functions and Definitions

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

// LCD Functions and Definitions

#define S5L87XX_LCDCON ((volatile struct s5l87xx_lcdcon *)0x38300000)

struct s5l87xx_lcdcon {
    uint32_t config; // 0x00
    uint32_t wcmd;   // 0x04
    uint32_t unk1;   // 0x08
    uint32_t rcmd;   // 0x0C
    uint32_t rdata;  // 0x10
    uint32_t dbuff;  // 0x14
    uint32_t intcon; // 0x18
    uint32_t status; // 0x1C
    uint32_t phtime; // 0x20
    uint32_t unk[4]; // 0x24
    uint32_t wdata;  // 0x40
};

enum s5l87xx_lcd_type {
    S5L87XX_LCD_TYPE_UNKNOWN = -1,
    S5L87XX_LCD_TYPE_38B3 = 0,
    S5L87XX_LCD_TYPE_38C4,
    S5L87XX_LCD_TYPE_38D5,
    S5L87XX_LCD_TYPE_38E6,
    S5L87XX_LCD_TYPE_58XX,
    N_LCD_TYPES
};

static void s5l_lcd_write_cmd(uint16_t cmd) {
    while (S5L87XX_LCDCON->status & 0x10);
    S5L87XX_LCDCON->wcmd = cmd;
}

static void s5l_lcd_recv_cmd8(uint8_t cmd, int len, uint8_t *buf) {
    s5l_lcd_write_cmd(cmd);
    while (len--) {
        udelay(100);
        while (!(S5L87XX_LCDCON->status & 0x2));
        S5L87XX_LCDCON->rdata = 0;
        while (!(S5L87XX_LCDCON->status & 1));
        *buf++ = S5L87XX_LCDCON->dbuff >> 1;
    }
}

static void s5l_lcd_write_config(uint32_t config) {
    while (!(S5L87XX_LCDCON->status & 0x2));
    udelay(1);
    S5L87XX_LCDCON->config = config;
}

static enum s5l87xx_lcd_type s5l87xx_lcdcon_get_type(void) {
    int retry = 3;
    uint8_t lcd_id[4];
    while (retry--) {
        s5l_lcd_write_config(0x80000c20);
        s5l_lcd_recv_cmd8(4, 4, lcd_id);

        if (lcd_id[1] == 0x58) return S5L87XX_LCD_TYPE_58XX;
        else if (lcd_id[1] == 0x38) {
            if      (lcd_id[2] == 0xb3) return S5L87XX_LCD_TYPE_38B3;
            else if (lcd_id[2] == 0xc4) return S5L87XX_LCD_TYPE_38C4;
            else if (lcd_id[2] == 0xd5) return S5L87XX_LCD_TYPE_38D5;
            else if (lcd_id[2] == 0xe6) return S5L87XX_LCD_TYPE_38E6;
        }
    }

    return S5L87XX_LCD_TYPE_UNKNOWN;
}

void s5l87xx_lcd_init(void) {
    debug("s5l87xx_lcd_init\n");
    s5l87xx_enable_clkgate("lcd");

    S5L87XX_LCDCON->config = 0x81100db8;
    S5L87XX_LCDCON->phtime = 0x33;
    
    enum s5l87xx_lcd_type type = s5l87xx_lcdcon_get_type();
    const char *types;
    switch (type) {
    case S5L87XX_LCD_TYPE_38B3: types = "38b3"; break;
    case S5L87XX_LCD_TYPE_38C4: types = "38c4"; break;
    case S5L87XX_LCD_TYPE_38D5: types = "38d5"; break;
    case S5L87XX_LCD_TYPE_38E6: types = "38e6"; break;
    case S5L87XX_LCD_TYPE_58XX: types = "58xx"; break;
    default:                    types = "unknown"; break;
    }
    debug("%s: detected LCD type %s (%d)\n", __func__, types, type);
}

// GPIO Functions and Definitions

#define PCON(i)       (*((uint32_t volatile*)(0x3cf00000 + ((i) << 5))))
#define PDAT(i)       (*((uint32_t volatile*)(0x3cf00004 + ((i) << 5))))
#define PUNA(i)       (*((uint32_t volatile*)(0x3cf00008 + ((i) << 5))))
#define PUNB(i)       (*((uint32_t volatile*)(0x3cf0000c + ((i) << 5))))
#define PUNC(i)       (*((uint32_t volatile*)(0x3cf00010 + ((i) << 5))))

void s5l87xx_gpio_init(void) {
    debug("s5l87xx_gpio_init\n");
    static uint32_t gpio_data[] = {
        0xE322222F, 0xEEEEEE00, 0x2332EEEE, 0x3333E222,
        0xEEE33333, 0x3EE0EEEE, 0x0F00EE33, 0xEEEEEEE0,
        0x22222222, 0x22222222, 0x33322222, 0xEEEEEEEE,
        0xEEEEEEEE, 0xEEEEEEEE, 0xEE2222EE, 0xEEEE0EEE,
    };

    for (int i = 0; i < 16; i++) {
        PCON(i) = gpio_data[i];
        PUNB(i) = 0;
        PUNC(i) = 0;
    }
}

// Buscon Functions and Definitions

struct s5l87xx_buscon {
    uint32_t unk[3];
    uint32_t remap;
};

enum s5l87xx_buscon_remap {
    S5L87XX_BUSCON_REMAP_ENABLE = 1,
    S5L87XX_BUSCON_REMAP_SRAM = 2,
};

static void s5l87xx_buscon_remap_sdram(void) {
    debug("s5l87xx_buscon_remap_sdram\n");
    volatile struct s5l87xx_buscon *buscon = (struct s5l87xx_buscon *)0x3E000000;
    buscon->remap = S5L87XX_BUSCON_REMAP_ENABLE;
}

// OTG PHY Functions and Definitions
// Register layout reverse-engineered from s5l8702 disk mode QEMU trace.

struct s5l87xx_otgphy {
    uint32_t pwr;        // 0x00 - power down control
    uint32_t clk;        // 0x04 - clock select
    uint32_t rstcon;     // 0x08 - reset control (3 bits)
    uint32_t pad0[3];    // 0x0c-0x14
    uint32_t bias;       // 0x18 - analog bias/trim
    uint32_t pad1[5];    // 0x1c-0x2c  (0x28 read-only by bootrom)
    uint32_t intfcon;    // 0x30 - interface control
    uint32_t pad2[3];    // 0x34-0x3c
    uint32_t phy_ctrl1;  // 0x40 - analog power stage 1
    uint32_t phy_ctrl2;  // 0x44 - analog power stage 2
};

static void s5l87xx_otgphy_off(void) {
    debug("s5l87xx_otgphy: turning off\n");
    volatile struct s5l87xx_otgphy *phy = (struct s5l87xx_otgphy *)0x3c400000;
    phy->phy_ctrl2 = 0;
    phy->phy_ctrl1 = 0;
    phy->rstcon = 0x7;
    phy->pwr = 0xff;
}

static void s5l87xx_otgphy_on(void) {
    debug("s5l87xx_otgphy: turning on\n");
    s5l87xx_enable_clkgate("usb-otg");
    s5l87xx_enable_clkgate("usb2-phy");

    volatile struct s5l87xx_otgphy *phy = (struct s5l87xx_otgphy *)0x3c400000;
    volatile uint32_t *phy_enable = (uint32_t *)0x3c400100;

    phy->pwr    = 0x000;
    phy->clk    = 0x000;
    phy->bias   = 0x400;
    phy->rstcon = 0x007;  // assert all three reset signals

    // Ramp up PHY analog stage 1 incrementally (from disk mode trace).
    phy->phy_ctrl1 = 0x300;
    phy->phy_ctrl1 = 0x340;
    phy->phy_ctrl1 = 0x346;
    phy->phy_ctrl1 = 0x347;

    // Ramp up PHY analog stage 2 incrementally (from disk mode trace).
    phy->phy_ctrl2 = 0x0c00;
    phy->phy_ctrl2 = 0x0fc0;
    phy->phy_ctrl2 = 0x0fe0;
    phy->phy_ctrl2 = 0x0ff0;
    phy->phy_ctrl2 = 0x0fff;

    *phy_enable = 1;      // enable PHY output

    phy->rstcon  = 0x000; // deassert reset
    phy->bias    = 0x400;
    phy->intfcon = 0x000;
    phy->bias    = 0x000;
}

void otg_phy_init(void *unused) {
    s5l87xx_otgphy_on();
}

void otg_phy_off(void *unused) {
    s5l87xx_otgphy_off();
}

// Timer Functions and Definitions

struct s5l87xx_timer {
    uint32_t con;     // 0x000
    uint32_t cmd;     // 0x004
    uint32_t data0;   // 0x008
    uint32_t data1;   // 0x00c
    uint32_t pre;     // 0x010
    uint32_t cnt;     // 0x014
};

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

int timer_init(void) {
    s5l87xx_timer_configure_interval(S5L87XX_TIMER_F);
    s5l87xx_timer_start(S5L87XX_TIMER_F);

    return 0;
}

unsigned long timer_read_counter(void) {
    return s5l87xx_timer_read(S5L87XX_TIMER_F);
}

int board_early_init_f(void) {
    s5l87xx_gpio_init();
    s5l87xx_lcd_init();

    // Linux needs timer3 ungated for pmctrl.
    s5l87xx_enable_clkgate("timer3");

    // Disable all VIC interrupts before U-Boot configures them.
    static volatile uint32_t *vic0_enclr = (uint32_t *)0x38e00014;
    static volatile uint32_t *vic1_enclr = (uint32_t *)0x38e01014;
    *vic0_enclr = 0xffffffff;
    *vic1_enclr = 0xffffffff;

    s5l87xx_enable_clkgate("usb-otg");
    s5l87xx_enable_clkgate("usb2-phy");

    // Disable USB power-clock gating so the DWC2 core is accessible.
    volatile uint32_t *pcgcctl = (uint32_t *)0x38400e00;
    *pcgcctl = 0;
    return 0;
}


#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void) {
    s5l87xx_enable_clkgate("uart0");

    static volatile uint32_t *gpio = (uint32_t *)0x3cf00000;
    *gpio &= 0xff00ffff;
    *gpio |= 0x00220000;
}
#endif
