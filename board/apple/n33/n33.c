#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/s5l87xx_common.h>

DECLARE_GLOBAL_DATA_PTR;

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

void reset_cpu(void)
{
    s5l87xx_reset_cpu();
}

ulong get_tbclk(void)
{
    // Corresponds to TIMER_F setup from s5l87xx.c.
    return 1000000;
}

int print_cpuinfo(void)
{
    printf("CPU: Samsung S5L8730\n");
    return 0;
}

void board_gpio_init(void) {
    uint32_t gpio = readl(S5L87XX_PCON(0));
    gpio &= 0xff00ffff;
    gpio |= 0x00220000;
    writel(gpio, S5L87XX_PCON(0));
}

void board_clock_init(void) {
    s5l87xx_enable_clkgate("uart0");

    // HACKHACKHACK add a pmctrl to linux
    // needed for timer c0..???
    s5l87xx_enable_clkgate("timer3");
    // HACKHACKHACK
}

void board_vic_init(void) {
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
    
    /* Enable UART clocking */
    s5l87xx_enable_clkgate("uart0");
}
#endif
