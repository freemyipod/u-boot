/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2026 The freemyipod team (freemyipod.org)
 */

#include <init.h>
#include <asm/arch-s5l87xx/s5l87xx-clk.h>
#include <asm/arch/lcdcon.h>

enum n46_lcd_type {
    N46_LCD_TYPE_UNSUPPORTED = -1,
    N46_LCD_TYPE_38B3 = 0,
    N46_LCD_TYPE_38C4,
    N46_LCD_TYPE_38D5,
    N46_LCD_TYPE_38E6,
    N46_LCD_TYPE_58XX,
    N46_LCD_TYPE_COUNT,
};

static const char *n46_lcd_type_names[] = {
    [N46_LCD_TYPE_38B3] = "38b3",
    [N46_LCD_TYPE_38C4] = "38c4",
    [N46_LCD_TYPE_38D5] = "38d5",
    [N46_LCD_TYPE_38E6] = "38e6",
    [N46_LCD_TYPE_58XX] = "58xx",
};

static const char *n46_lcd_type_unsupported = "unsupported";
static const char *n46_lcd_type_unknown = "unknown";

static enum n46_lcd_type n46_lcd_get_type(void)
{
    int retry = 3;
    uint8_t lcd_id[4] = {0};

    while (retry--) {
        s5l8702_lcdcon_write_config(0x80000c20);
        s5l8702_lcdcon_recv_cmd8(4, 4, lcd_id);

        if (lcd_id[1] == 0x58)
            return N46_LCD_TYPE_58XX;

        if (lcd_id[1] == 0x38) {
            if (lcd_id[2] == 0xb3)
                return N46_LCD_TYPE_38B3;

            if (lcd_id[2] == 0xc4)
                return N46_LCD_TYPE_38C4;

            if (lcd_id[2] == 0xd5)
                return N46_LCD_TYPE_38D5;

            if (lcd_id[2] == 0xe6)
                return N46_LCD_TYPE_38E6;
        }
    }

    return N46_LCD_TYPE_UNSUPPORTED;
}

static const char *n46_lcd_get_name(enum n46_lcd_type type)
{
    if (type == N46_LCD_TYPE_UNSUPPORTED) {
        return n46_lcd_type_unsupported;
    }

    if (type < N46_LCD_TYPE_UNSUPPORTED || type >= N46_LCD_TYPE_COUNT) {
        return n46_lcd_type_unknown;
    }

    return n46_lcd_type_names[type];
}

void n46_lcd_init(void)
{
    s5l87xx_enable_clkgate("lcd");

    s5l8702_lcdcon_write_config(0x81100db8);
    s5l8702_lcdcon_write_phtime(0x33);

    enum n46_lcd_type type = n46_lcd_get_type();
    const char *name = n46_lcd_get_name(type);
    printf("LCD:   Type %s (%d)\n", name, type);
}
