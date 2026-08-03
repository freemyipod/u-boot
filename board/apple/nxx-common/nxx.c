/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2026 The freemyipod team (freemyipod.org)
 */

#include <fdtdec.h>
#include <env.h>
#include <asm/global_data.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
    return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
    return fdtdec_setup_memory_banksize();
}

__weak int board_init(void)
{
    return 0;
}

#ifdef CONFIG_SET_DFU_ALT_INFO
void set_dfu_alt_info(char *interface, char *devstr)
{
    char buf[64];
    ulong start;
    ulong size;

    if (!env_get("dfu_alt_info")) {
        start = gd->ram_base;
        size = gd->ram_size - SZ_4M;
        snprintf(buf, sizeof(buf), "firmware ram 0x%08lx 0x%08lx", start, size);
        env_set("dfu_alt_info", buf);
    }
}
#endif
