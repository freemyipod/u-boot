#include <fdtdec.h>

int dram_init(void)
{
    return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
    return fdtdec_setup_memory_banksize();
}

__weak int board_init(void)
{
    return 0;
}
