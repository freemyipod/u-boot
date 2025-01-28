#include <asm/global_data.h>

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
    *(volatile int *)(0x3CF00140) |= 0x22000;
    *(volatile int *)(0x3CF00154) |= 0x18;
}

// UART RX line 0x02000
// UART TX line 0x20000

void board_clock_init(void) {
    // *(volatile int *)(0x3C500048) = 0x1BA585;
    *(volatile int *)(0x3C50004C) &= ~(1<<31); // 1<<31 UART(?)
    // *(volatile int *)(0x3C500058) = 0x1AF1;
    // *(volatile int *)(0x3C500068) = 0x3C0FC;
    *(volatile int *)(0x3C50006C) &= ~(1<<10);
}



#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	/* configuring UART TX & RX line GPIO */
    board_gpio_init();

    /* enabling UART clocking */
    board_clock_init();

	return 0;
}
#endif