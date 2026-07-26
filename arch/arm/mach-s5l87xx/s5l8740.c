#include <init.h>

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void) {
    printf("CPU: Samsung S5L8740 (ARM Cortex A5)\n");
    return 0;
}
#endif

void enable_caches(void)
{
    // Caches already enabled by the bootloader
    // This is here so that we don't see
    // WARNING: Caches not enabled
}

ulong get_tbclk(void)
{
    // Corresponds to TIMER_F setup from s5l87xx.c.
    return 1000000;
}
