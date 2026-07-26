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

static void board_gpio_init(void) {
    // UART0
    uint32_t gpio = readl(S5L87XX_PCON(0));
    gpio &= S5L87XX_PCON_PINS_CLEAR_FN(4, 5);
    gpio |= S5L87XX_PCON_PINS_SET_FN(4, 5, 0x2);
    writel(gpio, S5L87XX_PCON(0));
}

static void board_clock_init(void) {
    s5l87xx_enable_clkgate("uart0");

    // HACKHACKHACK add a pmctrl to linux
    // needed for timer c0..???
    s5l87xx_enable_clkgate("timer3");
    // HACKHACKHACK
}

static void board_vic_init(void) {
    // Disable all VIC interrupts.
    // TODO(q3k): disable VIC elsewhere
    writel(0xffffffff, S5L87XX_VICINTENCLEAR(0));
    writel(0xffffffff, S5L87XX_VICINTENCLEAR(1));
}

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
    /* Enable GPIO pins */
    board_gpio_init();

    /* Enable clocking */
    board_clock_init();
    
    /* Configure VIC */
    board_vic_init();

    return 0;
}
#endif

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
    /* Enable GPIO pins */
    board_gpio_init();
    
    /* Enable UART clocking. Runs before the FDT is set up, so ungate uart0 by
     * raw {gate, bit} ({1,9} + {9,7}) rather than the device-tree name lookup. */
    s5l87xx_enable_clkgate_bit(1, 9);
    s5l87xx_enable_clkgate_bit(9, 7);
}
#endif
