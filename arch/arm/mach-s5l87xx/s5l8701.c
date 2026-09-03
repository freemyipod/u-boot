/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2026 The freemyipod team (freemyipod.org)
 */

#include <init.h>

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void) {
    printf("CPU:   Samsung S5L8701 (ARM940T)\n");
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
    // Corresponds to timer setup from s5l8701-timer.c.
    // Is this correct?
    return 1416;
}
