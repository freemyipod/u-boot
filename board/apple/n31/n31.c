#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch-s5l87xx/s5l87xx.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
    return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
    return fdtdec_setup_memory_banksize();
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
    uint32_t value;

    // writel(0x1BA585, S5L87XX_PWRCON(0));
    
    value = readl(S5L87XX_PWRCON(1));
    value &= ~BIT(9);   // UART0
    value &= ~BIT(29);  // UART1
    value &= ~BIT(30);  // UART2
    value &= ~BIT(31);  // UART3
    writel(value, S5L87XX_PWRCON(1));

    // writel(0x1AF1, S5L87XX_PWRCON(2));
    // writel(0x3C0FC, S5L87XX_PWRCON(3));

    value = readl(S5L87XX_PWRCON(4));
    value &= ~BIT(7);   // UART0
    value &= ~BIT(8);   // UART1
    value &= ~BIT(9);   // UART2
    value &= ~BIT(10);  // UART3
    writel(value, S5L87XX_PWRCON(4));
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

    /* Enable UART clocking */
    board_clock_init();
}
#endif
