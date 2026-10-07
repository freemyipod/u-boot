// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026 The freemyipod team (freemyipod.org)
 */

#include <asm/io.h>
#include <asm/arch-s5l87xx/s5l87xx.h>
#include <linux/delay.h>
#include <linux/iopoll.h>
#include <command.h>
#include <vsprintf.h>

#define S5L8723_LCD_BASE		0x38300000

#define S5L8723_LCD_CON			0x00 /* Control register. */
#define S5L8723_LCD_WCMD		0x04 /* Write command register. */
#define S5L8723_LCD_RCMD		0x0C /* Read command register. */
#define S5L8723_LCD_RDATA		0x10 /* Read data register. */
#define S5L8723_LCD_DBUFF		0x14 /* Read Data buffer */
#define S5L8723_LCD_INTCON		0x18 /* Interrupt control register */
#define S5L8723_LCD_STATUS		0x1C /* LCD Interface status 0106 */
#define S5L8723_LCD_PHTIME		0x20 /* Phase time register 0060 */
#define S5L8723_LCD_RST_TIME	0x24 /* Reset active period 07FF */
#define S5L8723_LCD_DRV_RST		0x28 /* Reset drive signal */
#define S5L8723_LCD_WDATA		0x40 /* Write data register (0x40...0x5C) FIXME */

#define S5L8723_LCD_STATUS_READY	BIT(1)
#define S5L8723_LCD_STATUS_BUSY		BIT(4)

#define S5L8723_DSI_BASE			0x3d800000

#define S5L8723_DSI_00			0x00
#define S5L8723_DSI_04			0x04
#define S5L8723_DSI_08			0x08
#define S5L8723_DSI_10			0x10
#define S5L8723_DSI_14			0x14
#define S5L8723_DSI_18			0x18
#define S5L8723_DSI_28			0x28
#define S5L8723_DSI_2C			0x2c
#define S5L8723_DSI_30			0x30
#define S5L8723_DSI_WCMD			0x34
#define S5L8723_DSI_WDATA			0x38
#define S5L8723_DSI_3C			0x3c
#define S5L8723_DSI_40			0x40
#define S5L8723_DSI_STATUS			0x44
#define S5L8723_DSI_4C			0x4C
#define S5L8723_DSI_50			0x50
#define S5L8723_DSI_54			0x54
#define S5L8723_DSI_58			0x58
#define S5L8723_DSI_7C			0x7C

#define S5L8723_DSI_00_00_08			(BIT(8) | BIT(0))
#define S5L8723_DSI_00_09				BIT(9)
#define S5L8723_DSI_00_31				BIT(31)

#define S5L8723_DSI_STATUS_READY_WCMD	BIT(22)
#define S5L8723_DSI_STATUS_READY2		BIT(24)

#define S5L8723_DSI_2C_BUSY				BIT(31)
#define S5L8723_DSI_4C_INIT_DONE		BIT(23)

#define WIDTH 240
#define HEIGHT 240

static void ensure_mask(u32 addr, u32 mask, bool set, bool change)
{
	u32 val = readl(addr);

	if (((val & mask) > 0) == set)
		return;

	printf("%s failed: [0x%08x] == 0x%08x, 0x%08x & 0x%08x == 0x%08x, expected to be %u\n",
		__func__, addr, val, val, mask, val & mask, set);

	if (set) {
		val |= mask;
	}
	else {
		val &= ~mask;
	}

	if (change) {
		writel(val, addr);
		printf("%s: [0x%08x] set to 0x%08x\n", __func__, addr, val);
	}
}

static void ensure_val(u32 addr, u32 val, bool change)
{
	u32 oldval = readl(addr);

	if (oldval == val)
		return;

	printf("%s failed: [0x%08x] == 0x%08x, expected to be 0x%08x\n",
		__func__, addr, oldval, val);

	if (change) {
		writel(val, addr);
		printf("%s: [0x%08x] set to 0x%08x\n", __func__, addr, val);
	}
}

static int lcd_command(u32 cmd)
{
	printf("%s 0x%08x\n", __func__, cmd);

	writel(cmd, S5L8723_DSI_BASE + S5L8723_DSI_WCMD);

	u32 val;
	int result = readl_poll_timeout(
		S5L8723_DSI_BASE + S5L8723_DSI_STATUS,
		val,
		(val & S5L8723_DSI_STATUS_READY_WCMD) == S5L8723_DSI_STATUS_READY_WCMD,
		50000
	);

	if (result) {
		printf("timeout at %s: 0x%08x\n", __func__, cmd);
	}

	return result;
}

static int lcd_window(u32 cmd)
{
	printf("%s 0x%08x\n", __func__, cmd);

	/* Stock Dsim 0xaa4: generic-long, 5 bytes plus 20-byte zero padding. */
	clrbits_le32(S5L8723_DSI_BASE + S5L8723_DSI_10, BIT(28));

	writel(cmd, S5L8723_DSI_BASE + S5L8723_DSI_WDATA);
	writel(0xef, S5L8723_DSI_BASE + S5L8723_DSI_WDATA);

	for (int i = 0; i < 5; i++)
		writel(0, S5L8723_DSI_BASE + S5L8723_DSI_WDATA);

	int result = lcd_command(0x1929);

	if (result) {
		printf("timeout at %s: 0x%08x\n", __func__, cmd);
	}

	return result;
}

static int lcd_key(u32 value)
{
	u32 cmd = 0xf1 | (value << 8) | (value << 16);
	writel(cmd, S5L8723_DSI_BASE + S5L8723_DSI_WDATA);

	for (int i = 0; i < 5; i++)
		writel(0, S5L8723_DSI_BASE + S5L8723_DSI_WDATA);

	int result = lcd_command(0x1739);

	if (result) {
		printf("timeout at %s: value 0x%08x cmd 0x%08x\n", __func__, value, cmd);
	}

	return result;
}
#if 0
static int lcd_write_f4(const u8 original[14], bool sleep)
{
	u8 bytes[16] = { 0xf4 };

	for (int n = 0; n < 14; n++)
		bytes[n + 1] = original[n];

	if (sleep)
		bytes[8] = 0x0e; /* Native 0xc44 response byte7. */

	for (int n = 0; n < 4; n++) {
		u32 word = 0;

		for (int j = 0; j < 4; j++)
			word |= (u32)bytes[n * 4 + j] << (8 * j);

		writel(word, S5L8723_DSI_BASE + S5L8723_DSI_WDATA);
	}

	for (int i = 0; i < 5; i++)
		writel(0, S5L8723_DSI_BASE + S5L8723_DSI_WDATA);

	int result = lcd_command(0x2339);

	if (result) {
		printf("timeout at %s\n", __func__);
	}

	return result;
}
#endif
static int lcd_read_panel(u8 reg, unsigned length, u8 *bytes)
{
	u32 val;
	int result;

	if (!(readl(S5L8723_DSI_BASE + S5L8723_DSI_STATUS) & S5L8723_DSI_STATUS_READY2)) {
		return 1;
	}

	result = lcd_command((length << 8) | 0x37);

	if (result) {
		printf("%s: failed length %u\n", __func__, length);
		return result;
	}

	writel(GENMASK(31, 0), S5L8723_DSI_BASE + S5L8723_DSI_2C);

	result = lcd_command((reg << 8) | 0x06);

	if (result) {
		printf("%s: failed reg %u\n", __func__, reg);
		return result;
	}

	result = readl_poll_timeout(
		S5L8723_DSI_BASE + S5L8723_DSI_2C,
		val,
		val & 0x250003,
		50000
	);

	if (result) {
		printf("%s: timeout at reg %u: 0x%08x\n", __func__, reg, val);
		return result;
	}

	if (val & 0x210003) {
		printf("%s: reg check failed: 0x%08x\n", __func__, val);
		return 2;
	}

	result = readl_poll_timeout(
		S5L8723_DSI_BASE + S5L8723_DSI_STATUS,
		val,
		(val & S5L8723_DSI_STATUS_READY2) == S5L8723_DSI_STATUS_READY2,
		50000
	);

	if (result) {
		printf("timeout at %s: after reg check 0x%08x\n", __func__, val);
	}

	u32 header = readl(S5L8723_DSI_BASE + S5L8723_DSI_3C);
	unsigned type = header & 0x3f;
	unsigned size = (header >> 8) & 0xffff;

	if (type == 0x21 || type == 0x22 || type == 0x11 || type == 0x12) {
		size = (type == 0x21 || type == 0x11) ? 1 : 2;

		if (size != length) {
			printf("size %u != length %u\n", size, length);
			return 3;
		}

		for (int j = 0; j < size; j++) {
			bytes[j] = (header >> ((j + 1) * 8)) & 0xff;
		}

		return 0;
	}

	if ((type != 0x1c && type != 0x1a) || size != length) {
		printf("type mismatch 0x%02x or size %u != length %u\n", type, size, length);
		return 4;
	}

	for (int i = 0; i < (size + 3) / 4; i++) {
		result = readl_poll_timeout(
			S5L8723_DSI_BASE + S5L8723_DSI_STATUS,
			val,
			(val & S5L8723_DSI_STATUS_READY2) == S5L8723_DSI_STATUS_READY2,
			50000
		);

		if (result) {
			printf("timeout at %s: type 0x1c or 0x1a 0x%08x\n", __func__, val);
		}

		u32 word = readl(S5L8723_DSI_BASE + S5L8723_DSI_3C);

		for (int j = 0; j < 4 && i * 4 + j < size; j++)
			bytes[i * 4 + j] = word >> (j * 8);
	}

	return 0;
}

static int lcd_init(void)
{
	u32 val;
	int result;

	if (readl(S5L8723_DSI_BASE + S5L8723_DSI_4C) & S5L8723_DSI_4C_INIT_DONE) {
		printf("%s: already done, skipping\n", __func__);
		return 0;
	}

	if (readl(S5L8723_DSI_BASE + S5L8723_DSI_7C) < 16) {
		printf("%s: 0x7c < 16\n", __func__);
		return 0;
	}

	/* Dsim 0x870 / 0xde0. Cold controller, no LCDIF or panel commands. */
	writel(0xffffffff, S5L8723_DSI_BASE + S5L8723_DSI_30);
	writel(S5L8723_DSI_2C_BUSY, S5L8723_DSI_BASE + S5L8723_DSI_2C);

	if (readl(S5L8723_DSI_BASE + S5L8723_DSI_2C) & S5L8723_DSI_2C_BUSY) {
		printf("%s: 0x2c busy\n", __func__);
		return 0;
	}

	writel(0x0480c6e2, S5L8723_DSI_BASE + S5L8723_DSI_4C);
	writel(0xa25a8, S5L8723_DSI_BASE + S5L8723_DSI_50);

	result = readl_poll_timeout(
		S5L8723_DSI_BASE + S5L8723_DSI_00,
		val,
		(val & S5L8723_DSI_00_31),
		50000
	);

	if (result) {
		printf("timeout PLL wait\n");
		return result;
	}

	result = readl_poll_timeout(
		S5L8723_DSI_BASE + S5L8723_DSI_2C,
		val,
		(val & S5L8723_DSI_2C_BUSY),
		50000
	);

	if (result) {
		printf("timeout 0x2c busy\n");
		return result;
	}

	writel(0x11180002, S5L8723_DSI_BASE + S5L8723_DSI_08);
	writel(1, S5L8723_DSI_BASE + S5L8723_DSI_04);
	mdelay(1);

	writel(0, S5L8723_DSI_BASE + S5L8723_DSI_04);
	val = readl(S5L8723_DSI_BASE + S5L8723_DSI_00);
	writel(0xffffffff, S5L8723_DSI_BASE + S5L8723_DSI_30);
	/* 240x240, stream enable deliberately clear */
	writel(0x00f000f0, S5L8723_DSI_BASE + S5L8723_DSI_18);
	writel(0x2c00, S5L8723_DSI_BASE + S5L8723_DSI_54);
	writel(0, S5L8723_DSI_BASE + S5L8723_DSI_58);
	writel(10, S5L8723_DSI_BASE + S5L8723_DSI_28);
	writel(0x1ff, S5L8723_DSI_BASE + S5L8723_DSI_40);
	writel(0x1d, S5L8723_DSI_BASE + S5L8723_DSI_STATUS);

	/* Pixel-format property 6 -> switch helper target 0x8c2 -> format 7.
	 * One lane: (7<<12) | ((1-1)<<5) | ((1<<1)*2-2) | 0x700001. */
	writel(0x00707003, S5L8723_DSI_BASE + S5L8723_DSI_10);
	writel(BIT(20), S5L8723_DSI_BASE + S5L8723_DSI_14);
	mdelay(1);

	val = readl(S5L8723_DSI_BASE + S5L8723_DSI_14);
	val &= ~BIT(20);
	writel(val, S5L8723_DSI_BASE + S5L8723_DSI_14);

	if (readl(S5L8723_DSI_BASE + S5L8723_DSI_00) & S5L8723_DSI_00_09) {
		val = readl(S5L8723_DSI_BASE + S5L8723_DSI_14);
		val |= 0x5;
		writel(val, S5L8723_DSI_BASE + S5L8723_DSI_14);

		result = readl_poll_timeout(
			S5L8723_DSI_BASE + S5L8723_DSI_00,
			val,
			!(val & S5L8723_DSI_00_09),
			50000
		);

		if (result) {
			printf("timeout 0x00 0x09\n");
			return result;
		}
	}

	result = readl_poll_timeout(
		S5L8723_DSI_BASE + S5L8723_DSI_00,
		val,
		(val & S5L8723_DSI_00_00_08) == S5L8723_DSI_00_00_08,
		50000
	);

	if (result) {
		printf("timeout 0x00 0x101\n");
		return result;
	}

	val = readl(S5L8723_DSI_BASE + S5L8723_DSI_14);
	val |= 0xc0;
	writel(val, S5L8723_DSI_BASE + S5L8723_DSI_14);

	uint32_t z=0;
	__asm__ volatile("mcr p15, 0, %0, c7, c10, 0\n"
					 "mcr p15, 0, %0, c7, c10, 4"::"r"(z):"memory");

	return 0;
}

static int lcd_wake(void)
{
	ensure_mask(0x39700014,			BIT(2),		false, false);	// EIC
	ensure_mask(S5L87XX_PWRCON(0),	BIT(1),		false, false);	// clockgate 0, pin 1 LCD
	ensure_mask(S5L87XX_PWRCON(1),	BIT(19),	false, true);	// clockgate 1, pin 19 DSI

	ensure_val(S5L8723_LCD_BASE + S5L8723_LCD_CON,	0x801006b2, true);

	ensure_mask(S5L8723_DSI_BASE + S5L8723_DSI_00,	S5L8723_DSI_00_31, true, true);
	ensure_val(S5L8723_DSI_BASE + S5L8723_DSI_08,	0x91180002, true);
	ensure_val(S5L8723_DSI_BASE + S5L8723_DSI_10,	  0x707003, true);
	ensure_val(S5L8723_DSI_BASE + S5L8723_DSI_14,		   0x0, true);
	ensure_val(S5L8723_DSI_BASE + S5L8723_DSI_18,	0x80f000f0, true);

	int result;

	result = lcd_command(0x1105);

	if (result) {
		printf("%s: failed cmd %u\n", __func__, 0x1105);
		return result;
	}

	mdelay(120);

	u8 power;
	if (lcd_read_panel(0x0a, 1, &power))
		return 1;

	if (!(power & 0x10))
		return 2;

	// if f4 changed write it back

	if (lcd_key(0xa5))
		return 3;

	if (lcd_command(0x2905))
		return 4;

	mdelay(34);

	if (lcd_read_panel(0x0a, 1, &power))
		return 5;

	if ((power & 0x14) != 0x14)
		return 6;

	// pmic write turn on light

	return 0;
}
#if 0
static int lcd_sleep(void)
{
	u8 power;
	if (!lcd_read_panel(0x0a, 1, &power) || (power & 0x14) != 0x14) {
		/* The read request itself used DSI; do not retry an ambiguous FIFO. */
		return 1;
	}

	/*
	u32 light;
	if (pmic_read(0x26, &light) || light > 0xff)
		return 2;

	if (pmic_write(0x26, light & ~1u))
		return 3;

	u32 check;
	if (pmic_read(0x26, &check) || check != (light & ~1u))
		return 4;
	*/

	if (lcd_command(0x2805))
		return 5;

	mdelay(5);

	if (lcd_key(0x5a))
		return 6;

	u8 f4[14];

	if (lcd_read_panel(0xf4, 14, f4))
		return 7;

	if (lcd_write_f4(f4, true))
		return 8;

	if (lcd_key(0x5a))
		return 9;

	if (lcd_command(0x1005))
		return 10;

	mdelay(120);

	if (lcd_read_panel(0x0a, 1, &power))
		return 11;

	if (power & 0x10)
		return 12;

	return 0;
}
#endif
static int lcd_draw(u32 *src)
{
	u32 val;
	int result;

	ensure_mask(S5L8723_LCD_BASE + S5L8723_LCD_STATUS, S5L8723_LCD_STATUS_READY,	true, false);
	ensure_mask(S5L8723_DSI_BASE + S5L8723_DSI_STATUS, S5L8723_DSI_STATUS_READY2,	true, false);

	result = lcd_window(0x2a);

	if (result) {
		return result;
	}

	result = lcd_window(0x2b);

	if (result) {
		return result;
	}

	result = lcd_command(0x2c05);

	if (result) {
		return result;
	}

	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			result = readl_poll_timeout(
				S5L8723_LCD_BASE + S5L8723_LCD_STATUS,
				val,
				!(val & S5L8723_LCD_STATUS_BUSY),
				50000
			);

			if (result) {
				printf("timeout at x=%u y=%u\n", x, y);
				return result;
			}

			int bmp_y = HEIGHT - (y + 1);
			writel(src[bmp_y * WIDTH + x], S5L8723_LCD_BASE + S5L8723_LCD_WDATA);
		}
	}

	// drain
	result = readl_poll_timeout(
		S5L8723_LCD_BASE + S5L8723_LCD_STATUS,
		val,
		(val & S5L8723_LCD_STATUS_READY) == S5L8723_LCD_STATUS_READY,
		50000
	);

	if (result) {
		printf("timeout at drain\n");
		return result;
	}

	printf("done\n");

	return 0;
}

static int draw(struct cmd_tbl *cmdtp, int flag, int argc,
				char *const argv[])
{
	int result = lcd_init();

	if (result) {
		// cleanup
		writel(1, S5L8723_DSI_BASE + S5L8723_DSI_04);
		writel(0xffff, S5L8723_DSI_BASE + S5L8723_DSI_08);
		writel(0, S5L8723_DSI_BASE + S5L8723_DSI_4C);
		writel(S5L8723_DSI_2C_BUSY, S5L8723_DSI_BASE + S5L8723_DSI_2C);
		writel(0, S5L8723_DSI_BASE + S5L8723_DSI_04);

		printf("lcd_init: %d\n", result);
		return result;
	}

	result = lcd_wake();

	if (result) {
		printf("lcd_wake: %d\n", result);
		return result;
	}

	u32 *src = (u32 *)hextoul(argv[1], NULL);

	result = lcd_draw(src);

	if (result)
		printf("lcd_draw: %d\n", result);

	return result;
}

U_BOOT_CMD(draw, 2, 1, draw, "draw", "");
