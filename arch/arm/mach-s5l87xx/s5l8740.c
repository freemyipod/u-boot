#include <init.h>

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void) {
    printf("CPU: Samsung S5L8740 (ARM Cortex A5)\n");
    return 0;
}
#endif
