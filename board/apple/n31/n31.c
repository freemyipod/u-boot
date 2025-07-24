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
	printf("CPU: Samsung S5L8740\n");
	return 0;
}

void board_gpio_init(void) {
    uint32_t value;

    value = readl(S5L87XX_GPIO_BASE + 0x140);
    value |= 0x22000;
    writel(value, S5L87XX_GPIO_BASE + 0x140);
    
    value = readl(S5L87XX_GPIO_BASE + 0x154);
    value |= 0x18;
    writel(value, S5L87XX_GPIO_BASE + 0x154);
}

// UART RX line 0x02000
// UART TX line 0x20000

void board_clock_init(void) {
    uint32_t value;

    // writel(0x1BA585, S5L87XX_PWRCON(0));
    
    value = readl(S5L87XX_PWRCON(1));
    value &= ~(1<<31); // 1<<31 UART(?)
    writel(value, S5L87XX_PWRCON(1));

    // writel(0x1AF1, S5L87XX_PWRCON(2));
    // writel(0x3C0FC, S5L87XX_PWRCON(3));

    value = readl(S5L87XX_PWRCON(4));
    value &= ~(1<<10);
    writel(value, S5L87XX_PWRCON(4));
}

void enable_caches(void)
{
    // Caches already enabled by the bootloader
    // just here so that we don't see
	// WARNING: Caches not enabled\n"
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
