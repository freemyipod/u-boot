/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2026 The freemyipod team (freemyipod.org)
 */

#include <init.h>
#include <asm/io.h>
#include <asm/arch-s5l87xx/s5l87xx.h>

struct s5l87xx_buscon {
    uint32_t unk[3]; // 0x00, 0x04, 0x08
    uint32_t remap;  // 0x0c
};

enum s5l87xx_buscon_remap {
    S5L87XX_BUSCON_REMAP_ENABLE = 1,
    S5L87XX_BUSCON_REMAP_SRAM = 2,
};

static void s5l87xx_buscon_remap_sdram(void) {
    log_debug("s5l87xx_buscon_remap_sdram\n");
    volatile struct s5l87xx_buscon *buscon = (struct s5l87xx_buscon *)S5L87XX_BUS_BASE;
    writel(S5L87XX_BUSCON_REMAP_ENABLE, &buscon->remap);
}
