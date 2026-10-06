// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026 The freemyipod team (freemyipod.org)
 */

#include <init.h>
#include <asm/io.h>
#include <asm/arch-s5l87xx/s5l87xx.h>
#include <asm/arch-s5l87xx/s5l87xx-clk.h>
#include <linux/delay.h>

struct s5l87xx_otgphy {
	u32 pwr;	 // 0x00
	u32 con;	 // 0x04
	u32 rstcon;  // 0x08
	u32 unk[4];  // 0x0c, 0x10, 0x14, 0x18
	u32 unkcon;  // 0x1c
	u32 pad[36]; // 0x20 - 0x44
	u32 unk44;   // 0x44
};

void otg_phy_init(void *unused)
{
	struct s5l87xx_otgphy *otgphy = (struct s5l87xx_otgphy *)S5L87XX_PHY_BASE;

	log_debug("s5l8701_otgphy: turning on\n");
	s5l87xx_enable_clkgate("usb-otg");
	s5l87xx_enable_clkgate("usb2-phy");
	mdelay(10);

	// Disable USB suspend.
	writel(0, S5L87XX_OTG_BASE + 0xe00);

	writel(0, &otgphy->pwr); /* PHY: Power up */
	udelay(10);
	writel(1, &otgphy->unkcon);
	writel(0xe3f, &otgphy->unk44);
	writel(1, &otgphy->rstcon); /* PHY: Assert Software Reset */
	udelay(10);
	writel(0, &otgphy->rstcon); /* PHY: Deassert Software Reset */
	udelay(10);
	writel(0x600, &otgphy->unk[3]);
	writel(0, &otgphy->con);
	udelay(400);
}

void otg_phy_off(void *unused)
{
	struct s5l87xx_otgphy *otgphy = (struct s5l87xx_otgphy *)S5L87XX_PHY_BASE;

	log_debug("s5l8701_otgphy: turning off\n");
	writel(0x0F, &otgphy->pwr); /* PHY: Power down */
	udelay(10);
	writel(0x07, &otgphy->rstcon); /* PHY: Assert Software Reset */
	udelay(10);
}
