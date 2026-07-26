#include <init.h>

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void) {
    printf("CPU: Samsung S5L8702 (ARM926EJ-S)\n");
    return 0;
}
#endif

ulong get_tbclk(void)
{
    // Corresponds to TIMER_F setup from s5l87xx.c.
    return 1000000;
}
