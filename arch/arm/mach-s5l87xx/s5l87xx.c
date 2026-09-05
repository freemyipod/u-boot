/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2026 The freemyipod team (freemyipod.org)
 */

#include <init.h>
#include <asm/io.h>
#include <asm/arch-s5l87xx/s5l87xx.h>

void reset_cpu(void) {
    svc32_mode_en();
    // According to S5L8700X datasheet
    // rSWRCON = 0xA5 triggers a Software Reset
    // rWDTCON = 0x100000 is not documented but might trigger a Watchdog Reset
    // writel(0xA5, S5L87XX_SWRCON);

    writel(0x100000, S5L87XX_WDTCON);

    while (1)
        ;	/* loop forever till reset */
}
