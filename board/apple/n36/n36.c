/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2026 The freemyipod team (freemyipod.org)
 */

#include <asm/arch-s5l87xx/s5l87xx-clk.h>

static void board_clock_init(void) {
    s5l87xx_enable_clkgate("uart");
}

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
    /* enabling UART clocking */
    board_clock_init();

    return 0;
}
#endif
