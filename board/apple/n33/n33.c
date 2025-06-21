#include <asm/global_data.h>
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
    // Enable GPIO pins on N5G.
    static volatile uint32_t *gpio = (uint32_t *)S5L87XX_GPIOBASE_ADDR;
    *gpio &= 0xff00ffff;
    *gpio |= 0x00220000;
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
    static volatile uint32_t *vic0_enclr = (uint32_t *)(S5L87XX_VICBASE_ADDR + 0x14);
    static volatile uint32_t *vic1_enclr = (uint32_t *)(S5L87XX_VICBASE_ADDR + 0x14 + 0x1000);
    *vic0_enclr = 0xffffffff;
    *vic1_enclr = 0xffffffff;
}

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
    /* configuring UART TX & RX line GPIO */
    board_gpio_init();

    /* enabling UART clocking */
    board_clock_init();
    
    /* configure VIC */
    board_vic_init();

    return 0;
}
#endif
