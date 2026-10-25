// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026 The freemyipod team (freemyipod.org)
 */

#include <init.h>
#include <log.h>
#include <asm/io.h>
#include <asm/arch-s5l87xx/s5l87xx.h>
#include <asm/arch-s5l87xx/clk.h>
#include <linux/bitops.h>
#include <linux/delay.h>


struct s5l8740_otgphy {
	u32 pwr;		// 0x00 - PHYPWR
	u32 con;		// 0x04 - PHYCON
	u32 rstcon;		// 0x08 - URSTCON
	u32 pad0[4];	// 0x0c - 0x18
	u32 vldcon;		// 0x1c - UVLDCON
	u32 pad1[5];	// 0x20 - 0x30
	u32 rescon;		// 0x34 - URESCON
	u32 pad2[2];	// 0x38 - 0x3c
	u32 tune1;		// 0x40 - UOTGTUNE1
	u32 tune2;		// 0x44 - UOTGTUNE2
	u32 pad3[46];	// 0x48 - 0xfc
	u32 rsv;		// 0x100 - URSV
};

void otg_phy_init(void *unused)
{
	struct s5l8740_otgphy *otgphy = (struct s5l8740_otgphy *)S5L87XX_PHY_BASE;

	log_debug("s5l8740_otgphy: turning on\n");
	s5l87xx_enable_clkgate("usb-otg");
	s5l87xx_enable_clkgate("usb2-phy");

	// Stay detached until the gadget driver has configured the controller.
	setbits_le32(S5L87XX_OTG_BASE + 0x804, BIT(1)); /* DCTL: SftDiscon */
	mdelay(4);

	// Disable USB suspend: clear PCGCCTL GateHclk and StopPclk.
	clrbits_le32(S5L87XX_OTG_BASE + 0xe00, BIT(1) | BIT(0));
	udelay(100);

	writel(0, &otgphy->pwr); /* PHY: Power up */
	udelay(10);
	writel(1, &otgphy->con); /* 24 MHz reference clock */
	setbits_le32(&otgphy->rescon, BIT(2));
	writel(BIT(2), &otgphy->vldcon);

	// Tuning, written with the PLL and the PHY held in reset.
	writel(BIT(3) | BIT(0), &otgphy->rstcon);
	udelay(20);
	clrsetbits_le32(&otgphy->tune1, 0x7ff, 0x347);
	clrsetbits_le32(&otgphy->tune2, 0xfff, 0xe7f);
	setbits_le32(&otgphy->rsv, BIT(0));
	udelay(10);
	writel(0, &otgphy->rstcon);
	udelay(1000);

	writel(BIT(2) | BIT(1), &otgphy->vldcon);
}

void otg_phy_off(void *unused)
{
	struct s5l8740_otgphy *otgphy = (struct s5l8740_otgphy *)S5L87XX_PHY_BASE;

	log_debug("s5l8740_otgphy: turning off\n");
	// Enable USB suspend: set PCGCCTL GateHclk and StopPclk.
	setbits_le32(S5L87XX_OTG_BASE + 0xe00, BIT(1) | BIT(0));
	writel(0x1f, &otgphy->pwr); /* PHY: Power down */
	setbits_le32(&otgphy->rstcon, 7); /* PHY: Assert all resets */
	udelay(1000);
	// Should we gate the usb-otg and usb2-phy clocks?
}
