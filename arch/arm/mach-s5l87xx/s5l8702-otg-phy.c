#include <init.h>
#include <asm/io.h>
#include <asm/arch-s5l87xx/s5l87xx.h>
#include <asm/arch-s5l87xx/s5l87xx-clk.h>

// S5L8702 OTG PHY register layout, reverse-engineered from a disk mode QEMU
// trace. Differs from the S5L8730/N36 layout, so it is kept separate.
struct s5l8702_otgphy {
    uint32_t pwr;        // 0x00 - power down control
    uint32_t clk;        // 0x04 - clock select
    uint32_t rstcon;     // 0x08 - reset control (3 bits)
    uint32_t pad0[3];    // 0x0c-0x14
    uint32_t bias;       // 0x18 - analog bias/trim
    uint32_t pad1[5];    // 0x1c-0x2c  (0x28 read-only by bootrom)
    uint32_t intfcon;    // 0x30 - interface control
    uint32_t pad2[3];    // 0x34-0x3c
    uint32_t phy_ctrl1;  // 0x40 - analog power stage 1
    uint32_t phy_ctrl2;  // 0x44 - analog power stage 2
};

void otg_phy_init(void *unused) {
    log_debug("s5l8702_otgphy: turning on\n");
    s5l87xx_enable_clkgate("usb-otg");
    s5l87xx_enable_clkgate("usb2-phy");

    // Disable USB suspend.
	writel(0, S5L87XX_OTG_BASE + 0xe00);

    volatile struct s5l8702_otgphy *otgphy = (struct s5l8702_otgphy *)S5L87XX_PHY_BASE;

	writel(0x000, &otgphy->pwr);
	writel(0x000, &otgphy->clk);
	writel(0x400, &otgphy->bias);
	writel(0x007, &otgphy->rstcon); // assert all three reset signals

    // Ramp up PHY analog stage 1 incrementally (from disk mode trace).
	writel(0x300, &otgphy->phy_ctrl1);
	writel(0x340, &otgphy->phy_ctrl1);
	writel(0x346, &otgphy->phy_ctrl1);
	writel(0x347, &otgphy->phy_ctrl1);

    // Ramp up PHY analog stage 2 incrementally (from disk mode trace).
	writel(0x0c00, &otgphy->phy_ctrl2);
	writel(0x0fc0, &otgphy->phy_ctrl2);
	writel(0x0fe0, &otgphy->phy_ctrl2);
	writel(0x0ff0, &otgphy->phy_ctrl2);
	writel(0x0fff, &otgphy->phy_ctrl2);

	writel(1, S5L87XX_PHY_BASE + 0x100); // enable PHY output

	writel(0x000, &otgphy->rstcon); // deassert reset
	writel(0x400, &otgphy->bias);
	writel(0x000, &otgphy->intfcon);
	writel(0x000, &otgphy->bias);
}

void otg_phy_off(void *unused) {
	log_debug("s5l8702_otgphy: turning off\n");
	volatile struct s5l8702_otgphy *phy = (struct s5l8702_otgphy *)S5L87XX_PHY_BASE;
	writel(0, &phy->phy_ctrl2);
	writel(0, &phy->phy_ctrl1);
	writel(0x7, &phy->rstcon);
	writel(0xff, &phy->pwr);
}
