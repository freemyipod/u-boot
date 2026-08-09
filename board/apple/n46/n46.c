/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2026 The freemyipod team (freemyipod.org)
 */

#include <asm/io.h>
#include <asm/arch-s5l87xx/s5l87xx.h>
#include <asm/arch-s5l87xx/s5l87xx-clk.h>

void n46_lcd_init(void);

int board_init(void)
{
    n46_lcd_init();
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

static void board_clock_init(void)
{
    s5l87xx_enable_clkgate("uart0");

    // Linux needs the timer ungated for pmctrl.
    s5l87xx_enable_clkgate("timer");

    // I2C0 carries the PMU; Linux needs it clocked.
    s5l87xx_enable_clkgate("i2c0");

    s5l87xx_enable_clkgate("bootrom");
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
    // ({1,9} + {4,7}) rather than the device-tree-driven name lookup.
    s5l87xx_enable_clkgate_bit(1, 9);
    s5l87xx_enable_clkgate_bit(4, 7);
}
#endif
