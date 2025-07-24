#include <asm/global_data.h>
//#include <asm/io.h>
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
    // Corresponds to timer setup from s5l87xx.c.
    // Is this correct?
    return 1416;
}

int print_cpuinfo(void)
{
    printf("CPU: Samsung S5L8701\n");
    return 0;
}

void board_clock_init(void) {
    s5l87xx_enable_clkgate("uart");
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
    /* enabling UART clocking */
    board_clock_init();

    return 0;
}
#endif

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
    board_clock_init();
}
#endif
