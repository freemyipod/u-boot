#include <asm/global_data.h>
#include <asm/arch-s5l87xx/s5l87xx-clk.h>

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

static void board_clock_init(void) {
    s5l87xx_enable_clkgate("uart");
}

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
    /* enabling UART clocking */
    board_clock_init();

    return 0;
}
#endif
