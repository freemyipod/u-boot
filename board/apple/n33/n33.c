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

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
    debug("board_early_init_f\n");
    // HACKHACKHACK add a pmctrl to linux
    // needed for timer c0..???
    s5l87xx_enable_clkgate("timer3");
    // HACKHACKHACK

    // Disable all VIC interrupts.
    // TODO(q3k): disable VIC elsewhere
    static volatile uint32_t *vic0_enclr = (uint32_t *)0x38e00014;
    static volatile uint32_t *vic1_enclr = (uint32_t *)0x38e01014;
    *vic0_enclr = 0xffffffff;
    *vic1_enclr = 0xffffffff;

    s5l87xx_enable_clkgate("usb-otg");
    s5l87xx_enable_clkgate("usb2-phy");

    // Disable USB suspend. TODO(q3k): move this to DWC2?
    volatile uint32_t *pcgcctl = (uint32_t *)0x38400e00;
    *pcgcctl = 0;
    return 0;
}
#endif

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
    s5l87xx_enable_clkgate("uart0");

    // Enable GPIO pins on N5G.
    static volatile uint32_t *gpio = (uint32_t *)0x3cf00000;
    *gpio &= 0xff00ffff;
    *gpio |= 0x00220000;
}
#endif
