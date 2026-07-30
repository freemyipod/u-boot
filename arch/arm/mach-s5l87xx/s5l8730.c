#include <init.h>

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void) {
    printf("CPU:   Samsung S5L8730 (ARM1176JZF-S)\n");
    return 0;
}
#endif

void lowlevel_init(void)
{
    // unused
}

ulong get_tbclk(void)
{
    // Corresponds to TIMER_F setup from s5l87xx.c.
    return 1000000;
}
