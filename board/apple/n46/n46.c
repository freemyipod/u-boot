#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch-s5l87xx/s5l87xx.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

#define S5L87XX_PUNB(n) (S5L87XX_GPIO_BASE + 0x0c + (n) * 0x20)
#define S5L87XX_PUNC(n) (S5L87XX_GPIO_BASE + 0x10 + (n) * 0x20)

int dram_init(void)
{
    return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
    return fdtdec_setup_memory_banksize();
}

void lowlevel_init(void)
{
}

int board_init(void)
{
    return 0;
}

ulong get_tbclk(void)
{
    // Corresponds to TIMER_F setup from s5l87xx.c.
    return 1000000;
}

int print_cpuinfo(void)
{
    printf("CPU: Samsung S5L8702\n");
    return 0;
}

// Full GPIO pin-mux table for the iPod nano 3G, derived from the original
// firmware. Includes the UART0 pins, so this also brings up the debug UART.
static void board_gpio_init(void)
{
    static const uint32_t gpio_data[16] = {
        0xE322222F, 0xEEEEEE00, 0x2332EEEE, 0x3333E222,
        0xEEE33333, 0x3EE0EEEE, 0x0F00EE33, 0xEEEEEEE0,
        0x22222222, 0x22222222, 0x33322222, 0xEEEEEEEE,
        0xEEEEEEEE, 0xEEEEEEEE, 0xEE2222EE, 0xEEEE0EEE,
    };

    for (int i = 0; i < 16; i++) {
        writel(gpio_data[i], S5L87XX_PCON(i));
        writel(0, S5L87XX_PUNB(i));
        writel(0, S5L87XX_PUNC(i));
    }
}

// S5L8702 LCD controller register layout, reverse-engineered from the
// original firmware. Differs from the layout in s5l87xx.c.
struct s5l8702_lcdcon {
    uint32_t config; // 0x00
    uint32_t wcmd;   // 0x04
    uint32_t unk1;   // 0x08
    uint32_t rcmd;   // 0x0c
    uint32_t rdata;  // 0x10
    uint32_t dbuff;  // 0x14
    uint32_t intcon; // 0x18
    uint32_t status; // 0x1c
    uint32_t phtime; // 0x20
    uint32_t unk[4]; // 0x24
    uint32_t wdata;  // 0x40
};

#define N46_LCDCON ((volatile struct s5l8702_lcdcon *)S5L87XX_LCD_BASE)

enum n46_lcd_type {
    N46_LCD_TYPE_UNKNOWN = -1,
    N46_LCD_TYPE_38B3 = 0,
    N46_LCD_TYPE_38C4,
    N46_LCD_TYPE_38D5,
    N46_LCD_TYPE_38E6,
    N46_LCD_TYPE_58XX,
};

static void n46_lcd_write_cmd(uint16_t cmd)
{
    while (N46_LCDCON->status & 0x10)
        ;
    N46_LCDCON->wcmd = cmd;
}

static void n46_lcd_recv_cmd8(uint8_t cmd, int len, uint8_t *buf)
{
    n46_lcd_write_cmd(cmd);
    while (len--) {
        udelay(100);
        while (!(N46_LCDCON->status & 0x2))
            ;
        N46_LCDCON->rdata = 0;
        while (!(N46_LCDCON->status & 1))
            ;
        *buf++ = N46_LCDCON->dbuff >> 1;
    }
}

static void n46_lcd_write_config(uint32_t config)
{
    while (!(N46_LCDCON->status & 0x2))
        ;
    udelay(1);
    N46_LCDCON->config = config;
}

static enum n46_lcd_type n46_lcd_get_type(void)
{
    int retry = 3;
    uint8_t lcd_id[4];

    while (retry--) {
        n46_lcd_write_config(0x80000c20);
        n46_lcd_recv_cmd8(4, 4, lcd_id);

        if (lcd_id[1] == 0x58)
            return N46_LCD_TYPE_58XX;
        else if (lcd_id[1] == 0x38) {
            if      (lcd_id[2] == 0xb3) return N46_LCD_TYPE_38B3;
            else if (lcd_id[2] == 0xc4) return N46_LCD_TYPE_38C4;
            else if (lcd_id[2] == 0xd5) return N46_LCD_TYPE_38D5;
            else if (lcd_id[2] == 0xe6) return N46_LCD_TYPE_38E6;
        }
    }

    return N46_LCD_TYPE_UNKNOWN;
}

static void n46_lcd_init(void)
{
    s5l87xx_enable_clkgate("lcd");

    N46_LCDCON->config = 0x81100db8;
    N46_LCDCON->phtime = 0x33;

    enum n46_lcd_type type = n46_lcd_get_type();
    const char *types;
    switch (type) {
    case N46_LCD_TYPE_38B3: types = "38b3"; break;
    case N46_LCD_TYPE_38C4: types = "38c4"; break;
    case N46_LCD_TYPE_38D5: types = "38d5"; break;
    case N46_LCD_TYPE_38E6: types = "38e6"; break;
    case N46_LCD_TYPE_58XX: types = "58xx"; break;
    default:                types = "unknown"; break;
    }
    log_debug("n46_lcd_init: detected LCD type %s (%d)\n", types, type);
}

static void board_clock_init(void)
{
    s5l87xx_enable_clkgate("uart0");

    // Linux needs the timer ungated for pmctrl.
    s5l87xx_enable_clkgate("timer");

    // I2C0 carries the PMU; Linux needs it clocked.
    s5l87xx_enable_clkgate("i2c0");

    s5l87xx_enable_clkgate("usb-otg");
    s5l87xx_enable_clkgate("usb2-phy");

    // Disable USB power-clock gating so the DWC2 core is accessible.
    writel(0, S5L87XX_OTG_BASE + 0xe00);
}

static void board_vic_init(void)
{
    // Disable all VIC interrupts before U-Boot configures them.
    writel(0xffffffff, S5L87XX_VICINTENCLEAR(0));
    writel(0xffffffff, S5L87XX_VICINTENCLEAR(1));
}

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
    board_gpio_init();
    n46_lcd_init();
    board_clock_init();
    board_vic_init();

    return 0;
}
#endif

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
    // Mux the UART0 RX/TX pins (bank 0, pins 4/5) before the debug UART runs.
    uint32_t gpio = readl(S5L87XX_PCON(0));
    gpio &= S5L87XX_PCON_PINS_CLEAR_FN(4, 5);
    gpio |= S5L87XX_PCON_PINS_SET_FN(4, 5, 0x2);
    writel(gpio, S5L87XX_PCON(0));

    // Runs before the FDT is set up, so ungate uart0 by raw {gate, bit}
    // ({1,9} + {9,7}) rather than the device-tree-driven name lookup.
    s5l87xx_enable_clkgate_bit(1, 9);
    s5l87xx_enable_clkgate_bit(9, 7);
}
#endif
