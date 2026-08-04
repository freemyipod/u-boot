/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2026 The freemyipod team (freemyipod.org)
 */

#include <init.h>
#include <asm/io.h>
#include <asm/arch-s5l87xx/s5l87xx.h>
#include <linux/delay.h>

// S5L8702 LCD controller register layout, reverse-engineered from the
// original firmware. Differs from the layout in s5l87xx.
struct s5l8702_lcdcon {
    uint32_t config; // 0x00
    uint32_t wcmd;   // 0x04
    uint32_t unk1;   // 0x08
    uint32_t rcmd;   // 0x0c
    uint32_t rdata;  // 0x10
    uint32_t dbuff;  // 0x14
    uint32_t intcon; // 0x18
    uint32_t status; // 0x1c
    uint32_t phtime; // 0x20
    uint32_t unk[4]; // 0x24
    uint32_t wdata;  // 0x40
};

#define S5L8702_LCDCON ((volatile struct s5l8702_lcdcon *)S5L87XX_LCD_BASE)

static void s5l8702_lcdcon_write_cmd(uint16_t cmd)
{
    while (readl(&S5L8702_LCDCON->status) & 0x10)
        ;
    writel(cmd, &S5L8702_LCDCON->wcmd);
}

void s5l8702_lcdcon_recv_cmd8(uint8_t cmd, int len, uint8_t *buf)
{
    s5l8702_lcdcon_write_cmd(cmd);
    while (len--) {
        udelay(100);
        while (!(readl(&S5L8702_LCDCON->status) & 0x2))
            ;
        writel(0, &S5L8702_LCDCON->rdata);
        while (!(readl(&S5L8702_LCDCON->status) & 0x1))
            ;
        *buf++ = readl(&S5L8702_LCDCON->dbuff) >> 1;
    }
}

void s5l8702_lcdcon_write_config(uint32_t config)
{
    while (!(readl(&S5L8702_LCDCON->status) & 0x2))
        ;
    udelay(1);
    writel(config, &S5L8702_LCDCON->config);
}

void s5l8702_lcdcon_write_phtime(uint32_t phtime)
{
    writel(phtime, &S5L8702_LCDCON->phtime);
}
