#include <common.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
    if (fdtdec_setup_mem_size_base() != 0)
        return -EINVAL;

    return 0;
}

int dram_init_banksize(void)
{
    fdtdec_setup_memory_banksize();

    return 0;
}

int board_init(void)
{
    return 0;
}

void reset_cpu(void)
{
}

ulong get_tbclk(void)
{
    // Corresponds to TIMER_F setup from s5l87xx.c.
    return 1000000;
}
