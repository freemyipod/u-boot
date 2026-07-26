#include <init.h>

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void) {
    printf("CPU: Samsung S5L8702 (ARM926EJ-S)\n");
    return 0;
}
#endif
