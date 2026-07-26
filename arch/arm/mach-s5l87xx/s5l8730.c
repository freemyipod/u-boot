#include <init.h>

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void) {
    printf("CPU: Samsung S5L8730 (ARM1176JZF-S)\n");
    return 0;
}
#endif
