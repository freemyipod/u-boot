#include <init.h>
#include <asm/io.h>
#include <dm/ofnode.h>
#include <linux/delay.h>
#include <asm/arch/s5l87xx_common.h>

DECLARE_GLOBAL_DATA_PTR;

typedef struct {
    uint32_t pad[18];
    uint32_t gates[10];
} s5l87xx_clkcon;

#define S5L87XX_CLKCON ((volatile s5l87xx_clkcon *)S5L87XX_CLK_BASE)

/*
 * Sentinel for an unused second clock gate in the device-tree
 * "samsung,clock-gates" property. A real gate is encoded as gate * 32 + bit.
 */
#define S5L87XX_CLKGATE_NONE 0xffffffff

void s5l87xx_reset_cpu(void) {
    svc32_mode_en();
    // According to S5L8700X datasheet
    // rSWRCON = 0xA5 triggers a Software Reset
    // rWDTCON = 0x100000 is not documented but might trigger a Watchdog Reset
    // writel(0xA5, S5L87XX_SWRCON);

#if IS_ENABLED(CONFIG_TARGET_N36)
    writel(0x110AFF, S5L87XX_WDTCON);
    writel(0xff0, S5L87XX_WDTCNT);
    writel(0x1100FF, S5L87XX_WDTCON);
#else
    writel(0x100000, S5L87XX_WDTCON);
#endif

    while (1)
        ;	/* loop forever till reset */
}

void s5l87xx_enable_clkgate_bit(uint8_t gate, uint8_t bit) {
    uint32_t mask = ~(((uint32_t) 1) << bit);
#if IS_ENABLED(CONFIG_TARGET_N36)
    uint32_t value = readl(S5L87XX_PWRCON(gate));
    value &= mask;
    writel(value, S5L87XX_PWRCON(gate));
#else
    S5L87XX_CLKCON->gates[gate] &= mask;
#endif
}

static void s5l87xx_ungate_encoded(uint32_t encoded) {
    if (encoded == S5L87XX_CLKGATE_NONE)
        return;
    s5l87xx_enable_clkgate_bit(encoded / 32, encoded % 32);
}

/*
 * Ungate a clock by name. The name -> {gate, bit} mapping lives in the device
 * tree under the "samsung,s5l87xx-clkgates" node ("clock-gate-names" paired
 * with "samsung,clock-gates", two cells per gate). Works pre-relocation: it
 * reads the flat tree directly, so callers earlier than the FDT setup (e.g.
 * the debug UART) must use s5l87xx_enable_clkgate_bit() instead.
 */
void s5l87xx_enable_clkgate(const char *id) {
    ofnode node = ofnode_by_compatible(ofnode_null(),
                                       "samsung,s5l87xx-clkgates");
    if (!ofnode_valid(node))
        panic("s5l87xx_enable_clkgate: no clkgates node in device tree");

    int idx = ofnode_stringlist_search(node, "clock-gate-names", id);
    if (idx < 0)
        panic("s5l87xx_enable_clkgate: unknown id %s", id);

    uint32_t gate1, gate2;
    if (ofnode_read_u32_index(node, "samsung,clock-gates", idx * 2, &gate1) ||
        ofnode_read_u32_index(node, "samsung,clock-gates", idx * 2 + 1, &gate2))
        panic("s5l87xx_enable_clkgate: malformed gates for %s", id);

    log_debug("s5l87xx: ungating %s\n", id);
    s5l87xx_ungate_encoded(gate1);
    s5l87xx_ungate_encoded(gate2);
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
    uint32_t con;     // 0x00
    uint32_t cmd;     // 0x04
    uint32_t data0;   // 0x08
    uint32_t data1;   // 0x0c
    uint32_t pre;     // 0x10
    uint32_t cnt;     // 0x14
};

struct s5l87xx_otgphy {
    uint32_t pwr;     // 0x00
    uint32_t con;     // 0x04
    uint32_t rstcon;  // 0x08
    uint32_t unk[4];  // 0x0c, 0x10, 0x14, 0x18
    uint32_t unkcon;  // 0x1c
    uint32_t pad[36]; // 0x20 - 0x44
    uint32_t unk44;   // 0x44
};

struct s5l87xx_buscon {
    uint32_t unk[3]; // 0x00, 0x04, 0x08
    uint32_t remap;  // 0x0c
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

#define S5L87XX_LCDCON ((volatile struct s5l87xx_lcdcon *)S5L87XX_LCD_BASE)

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

static void s5l87xx_lcdcon_wait_ready(void) {
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
    case S5L87XX_LCD_TYPE_UNSUPPORTED:
        types = "UNSUPPORTED";
    }
    log_debug("%s: detected LCD type %s (%d)\n", __func__, types, type);
}

enum s5l87xx_buscon_remap {
    S5L87XX_BUSCON_REMAP_ENABLE = 1,
    S5L87XX_BUSCON_REMAP_SRAM = 2,
};

// unused
#if 0
static void s5l87xx_buscon_remap_sdram(void) {
    log_debug("s5l87xx_buscon_remap_sdram\n");
    volatile struct s5l87xx_buscon *buscon = (struct s5l87xx_buscon *)S5L87XX_BUS_BASE;
    buscon->remap = S5L87XX_BUSCON_REMAP_ENABLE;
}
#endif

#if IS_ENABLED(CONFIG_TARGET_N46)

// S5L8702 OTG PHY register layout, reverse-engineered from a disk mode QEMU
// trace. Differs from the S5L8730/N36 layout, so it is kept separate.
struct s5l8702_otgphy {
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
    log_debug("s5l87xx_otgphy: turning off\n");
    volatile struct s5l8702_otgphy *phy = (struct s5l8702_otgphy *)S5L87XX_PHY_BASE;
    phy->phy_ctrl2 = 0;
    phy->phy_ctrl1 = 0;
    phy->rstcon = 0x7;
    phy->pwr = 0xff;
}

static void s5l87xx_otgphy_on(void) {
    log_debug("s5l87xx_otgphy: turning on\n");
    s5l87xx_enable_clkgate("usb-otg");
    s5l87xx_enable_clkgate("usb2-phy");

    // Disable USB suspend.
    volatile uint32_t *pcgcctl = (uint32_t *)(S5L87XX_OTG_BASE + 0xe00);
    *pcgcctl = 0;

    volatile struct s5l8702_otgphy *phy = (struct s5l8702_otgphy *)S5L87XX_PHY_BASE;
    volatile uint32_t *phy_enable = (uint32_t *)(S5L87XX_PHY_BASE + 0x100);

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

#else

static void s5l87xx_otgphy_off(void) {
    log_debug("s5l87xx_otgphy: turning off\n");
    volatile struct s5l87xx_otgphy *otgphy = (struct s5l87xx_otgphy *)S5L87XX_PHY_BASE;
#if IS_ENABLED(CONFIG_TARGET_N36)
    otgphy->pwr = 0x0f;    /* PHY: Power down */
    udelay(10);
    otgphy->rstcon = 0x07; /* PHY: Assert Software Reset */
    udelay(10);
#else
    otgphy->pwr = 0xff;
    mdelay(10);
    otgphy->rstcon = 0xff;
    mdelay(10);
    otgphy->unkcon = 4;
#endif
}

static void s5l87xx_otgphy_on(void) {
    log_debug("s5l87xx_otgphy: turning on\n");
    s5l87xx_enable_clkgate("usb-otg");
    s5l87xx_enable_clkgate("usb2-phy");
    mdelay(10);

    // Disable USB suspend.
    // TODO(q3k): move this to DWC2?
    volatile uint32_t *pcgcctl = (uint32_t *)(S5L87XX_OTG_BASE + 0xe00);
    *pcgcctl = 0;

    volatile struct s5l87xx_otgphy *otgphy = (struct s5l87xx_otgphy *)S5L87XX_PHY_BASE;
    otgphy->pwr = 0; /* PHY: Power up */
#if IS_ENABLED(CONFIG_TARGET_N36)
    udelay(10);
    otgphy->unkcon = 1;
    otgphy->unk44 = 0xe3f;
    otgphy->rstcon = 1; /* PHY: Assert Software Reset */
    udelay(10);
    otgphy->rstcon = 0; /* PHY: Deassert Software Reset */
    udelay(10);
    otgphy->unk[3] = 0x600;
    otgphy->con = 0;
    udelay(400);
#else
    mdelay(10);
    otgphy->rstcon = 1;
    mdelay(10);
    otgphy->rstcon = 0;
    mdelay(10);
    otgphy->unkcon = 6;
    otgphy->con = 1;
    mdelay(400);
#endif
}

#endif

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
#if !IS_ENABLED(CONFIG_TARGET_N36)
    // Timer E: 64-bit (unimplemented, different registers from others)
    S5L87XX_TIMER_E = 4,
    // Timers F, G, H, I: 32-bit
    S5L87XX_TIMER_F = 5,
    S5L87XX_TIMER_G = 6,
    S5L87XX_TIMER_H = 7,
    S5L87XX_TIMER_I = 8,
#endif
};

#if IS_ENABLED(CONFIG_TARGET_N36)
#define SYSTEM_TIMER S5L87XX_TIMER_C
#else
#define SYSTEM_TIMER S5L87XX_TIMER_F
#endif

enum s5l87xx_timer_cmd {
    S5L87XX_TIMER_CMD_STOP = 0,
    S5L87XX_TIMER_CMD_START = 1,
    S5L87XX_TIMER_CMD_CLR = 2,
};

static struct s5l87xx_timer *s5l87xx_timer_registers(enum s5l87xx_timer_id id) {
    switch (id) {
    case S5L87XX_TIMER_A:
        return (struct s5l87xx_timer *)S5L87XX_TIMER_BASE;
    case S5L87XX_TIMER_B:
        return (struct s5l87xx_timer *)(S5L87XX_TIMER_BASE + 0x20);
    case S5L87XX_TIMER_C:
        return (struct s5l87xx_timer *)(S5L87XX_TIMER_BASE + 0x40);
    case S5L87XX_TIMER_D:
        return (struct s5l87xx_timer *)(S5L87XX_TIMER_BASE + 0x60);
#if !IS_ENABLED(CONFIG_TARGET_N36)
    case S5L87XX_TIMER_E:
        return (struct s5l87xx_timer *)(S5L87XX_TIMER_BASE + 0x80);
    case S5L87XX_TIMER_F:
        return (struct s5l87xx_timer *)(S5L87XX_TIMER_BASE + 0xa0);
    case S5L87XX_TIMER_G:
        return (struct s5l87xx_timer *)(S5L87XX_TIMER_BASE + 0xc0);
    case S5L87XX_TIMER_H:
        return (struct s5l87xx_timer *)(S5L87XX_TIMER_BASE + 0xe0);
    case S5L87XX_TIMER_I:
        return (struct s5l87xx_timer *)(S5L87XX_TIMER_BASE + 0x100);
#endif
    default:
        panic("requested invalid timer id %d", id);
    }
}

static const char* s5l87xx_timer_clockgate(enum s5l87xx_timer_id id) {
#if IS_ENABLED(CONFIG_TARGET_N36) || IS_ENABLED(CONFIG_TARGET_N46)
    /* S5L8702 gates all timers behind a single clock gate. */
    return "timer";
#else
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
#endif
}

static void s5l87xx_timer_configure_interval(enum s5l87xx_timer_id id) {
    log_debug("s5l87xx_timer: configuring %d in interval mode\n", id);
    s5l87xx_enable_clkgate(s5l87xx_timer_clockgate(id));

    volatile struct s5l87xx_timer *timer = s5l87xx_timer_registers(id);

    timer->cmd = S5L87XX_TIMER_CMD_STOP;
#if IS_ENABLED(CONFIG_TARGET_N36)
    /* configure timer for 1000 Hz??? */
    timer->con = (3 << 8) | (1 << 4);
    timer->pre = 511;
    timer->data0 = 0xffff;
    timer->data1 = 0xffff;
#else
    timer->con = 0x40;
    timer->pre = 0xb;
    timer->data0 = 0xffffffff;
#endif
    timer->cmd = S5L87XX_TIMER_CMD_CLR;
}

static void s5l87xx_timer_start(enum s5l87xx_timer_id id) {
    log_debug("s5l87xx_timer: starting %d\n", id);
    volatile struct s5l87xx_timer *timer = s5l87xx_timer_registers(id);
    timer->cmd = S5L87XX_TIMER_CMD_START;
}

// unused
#if 0
static void s5l87xx_timer_stop(enum s5l87xx_timer_id id) {
    log_debug("s5l87xx_timer: stopping %d\n", id);
    volatile struct s5l87xx_timer *timer = s5l87xx_timer_registers(id);
    timer->cmd = S5L87XX_TIMER_CMD_STOP;
}
#endif

static uint32_t s5l87xx_timer_read(enum s5l87xx_timer_id id) {
    volatile struct s5l87xx_timer *timer = s5l87xx_timer_registers(id);
    return timer->cnt;
}

int timer_init(void)
{
    s5l87xx_timer_configure_interval(SYSTEM_TIMER);
    s5l87xx_timer_start(SYSTEM_TIMER);

    return 0;
}

unsigned long timer_read_counter(void)
{
#if IS_ENABLED(CONFIG_TARGET_N36)
    static uint16_t last = 0;
    static uint16_t high = 0;
    
    uint16_t now = s5l87xx_timer_read(SYSTEM_TIMER);

    if (last > now) {
        high++;
    }

    last = now;

    return ((uint32_t)high << 16) | (uint32_t)now;
#else
    return s5l87xx_timer_read(SYSTEM_TIMER);
#endif
}
