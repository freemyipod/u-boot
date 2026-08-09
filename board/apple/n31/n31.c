/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2026 The freemyipod team (freemyipod.org)
 */

#include <asm/io.h>
#include <asm/arch-s5l87xx/s5l87xx.h>
#include <asm/arch-s5l87xx/s5l87xx-clk.h>

static void board_gpio_init(void) {
    uint32_t value;

    // UART0: pad 0, rx pin 4, tx pin 5, fn 2
    value = readl(S5L87XX_PCON(0));
    value &= S5L87XX_PCON_PINS_CLEAR_FN(4, 5);
    value |= S5L87XX_PCON_PINS_SET_FN(4, 5, 0x2);
    writel(value, S5L87XX_PCON(0));
    
    value = readl(S5L87XX_PPIE(0));
    value |= BIT(4) | BIT(5);
    writel(value, S5L87XX_PPIE(0));

    // UART1: pad 9, rx pin 6, tx pin 7, fn 2
    value = readl(S5L87XX_PCON(9));
    value &= S5L87XX_PCON_PINS_CLEAR_FN(6, 7);
    value |= S5L87XX_PCON_PINS_SET_FN(6, 7, 0x2);
    writel(value, S5L87XX_PCON(9));

    value = readl(S5L87XX_PPIE(9));
    value |= BIT(6) | BIT(7);
    writel(value, S5L87XX_PPIE(9));

    // UART2: pad 8, rx pin 2, tx pin 3, fn 2
    value = readl(S5L87XX_PCON(10));
    value &= S5L87XX_PCON_PINS_CLEAR_FN(2, 3);
    value |= S5L87XX_PCON_PINS_SET_FN(2, 3, 0x2);
    writel(value, S5L87XX_PCON(10));

    value = readl(S5L87XX_PPIE(10));
    value |= BIT(2) | BIT(3);
    writel(value, S5L87XX_PPIE(10));

    // UART3: pad 10, rx pin 3, tx pin 4, fn 2
    value = readl(S5L87XX_PCON(10));
    value &= S5L87XX_PCON_PINS_CLEAR_FN(3, 4);
    value |= S5L87XX_PCON_PINS_SET_FN(3, 4, 0x2);
    writel(value, S5L87XX_PCON(10));

    value = readl(S5L87XX_PPIE(10));
    value |= BIT(3) | BIT(4);
    writel(value, S5L87XX_PPIE(10));
}

// UART RX line 0x02000
// UART TX line 0x20000

static void board_clock_init(void) {
    s5l87xx_enable_clkgate("uart0");
    s5l87xx_enable_clkgate("uart1");
    s5l87xx_enable_clkgate("uart2");
    s5l87xx_enable_clkgate("uart3");
    s5l87xx_enable_clkgate("bootrom");
}

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	/* Configure UART TX & RX line GPIO */
    board_gpio_init();

    /* Enable UART clocking */
    board_clock_init();

	return 0;
}
#endif

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
    /* Configure UART TX & RX line GPIO */
    board_gpio_init();

    /* Enable UART clocking. Runs before the FDT is set up */
    // UART0
    s5l87xx_enable_clkgate_bit(1, 9);
    s5l87xx_enable_clkgate_bit(4, 7);

    // UART1
    s5l87xx_enable_clkgate_bit(1, 29);
    s5l87xx_enable_clkgate_bit(4, 8);

    // UART2
    s5l87xx_enable_clkgate_bit(1, 30);
    s5l87xx_enable_clkgate_bit(4, 9);

    // UART3
    s5l87xx_enable_clkgate_bit(1, 31);
    s5l87xx_enable_clkgate_bit(4, 10);
}
#endif
