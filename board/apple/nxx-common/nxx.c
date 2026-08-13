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
    char buf[96];
    ulong start;
    ulong size;

    if (!env_get("dfu_alt_info")) {
        /* don't touch the upper 4M, U-boot is relocated there
         * use upper half of the remaining space for loading
         * and the lower for booting the payload, e.g. Linux
         */
        size = (gd->ram_size - SZ_4M) / 2;
        start = gd->ram_base + size;
        snprintf(buf, sizeof(buf), "firmware ram 0x%08lx 0x%08lx;bootrom ram 0x20000000 0xc800", start, size);
        env_set("dfu_alt_info", buf);

        env_set_hex("loadaddr", start);
    }
}
#endif
