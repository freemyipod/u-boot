/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2026 The freemyipod team (freemyipod.org)
 */

#include <init.h>
#include <asm/io.h>
#include <asm/arch-s5l87xx/s5l87xx.h>
#include <asm/arch-s5l87xx/s5l87xx-clk.h>
#include <linux/delay.h>

struct s5l87xx_otgphy {
    uint32_t pwr;     // 0x00
    uint32_t con;     // 0x04
    uint32_t rstcon;  // 0x08
    uint32_t unk[4];  // 0x0c, 0x10, 0x14, 0x18
    uint32_t unkcon;  // 0x1c
    uint32_t pad[36]; // 0x20 - 0x44
    uint32_t unk44;   // 0x44
};

void otg_phy_init(void *unused) {
    log_debug("s5l87xx_otgphy: turning on\n");
    s5l87xx_enable_clkgate("usb-otg");
    s5l87xx_enable_clkgate("usb2-phy");
    mdelay(10);

    // Disable USB suspend.
    // TODO(q3k): move this to DWC2?
	writel(0, S5L87XX_OTG_BASE + 0xe00);

    volatile struct s5l87xx_otgphy *otgphy = (struct s5l87xx_otgphy *)S5L87XX_PHY_BASE;
	writel(0, &otgphy->pwr); /* PHY: Power up */
    mdelay(10);
    writel(1, &otgphy->rstcon);
    mdelay(10);
    writel(0, &otgphy->rstcon);
    mdelay(10);
	writel(6, &otgphy->unkcon);
	writel(1, &otgphy->con);
    mdelay(400);
}

void otg_phy_off(void *unused) {
	log_debug("s5l87xx_otgphy: turning off\n");
	volatile struct s5l87xx_otgphy *otgphy = (struct s5l87xx_otgphy *)S5L87XX_PHY_BASE;
	writel(0xFF, &otgphy->pwr);
	mdelay(10);
	writel(0xFF, &otgphy->rstcon);
	mdelay(10);
	writel(4, &otgphy->unkcon);
}
