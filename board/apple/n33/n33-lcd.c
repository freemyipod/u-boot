/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2026 The freemyipod team (freemyipod.org)
 */

#include <init.h>
#include <asm/arch/lcdcon.h>

enum n33_lcd_type {
    N33_LCD_TYPE_UNSUPPORTED = -1,
    N33_LCD_TYPE_38B3 = 0,
    N33_LCD_TYPE_38C4,
    N33_LCD_TYPE_38F7,
    N33_LCD_TYPE_48C4,
    N33_LCD_TYPE_COUNT,
};

static const char *n33_lcd_type_names[] = {
    [N33_LCD_TYPE_38B3] = "38b3",
    [N33_LCD_TYPE_38C4] = "38c4",
    [N33_LCD_TYPE_38F7] = "38f7",
    [N33_LCD_TYPE_48C4] = "48c4",
};

static const char *n33_lcd_type_unsupported = "unsupported";
static const char *n33_lcd_type_unknown = "unknown";

static enum n33_lcd_type n33_lcd_get_type(void) {
    uint8_t lcd_id[3] = {0};
    s5l8730_lcdcon_transact_read(4, 3, lcd_id);

    if (lcd_id[0] == 0x38) {
        if (lcd_id[1] == 0xb3) {
            return N33_LCD_TYPE_38B3;
        }

        if (lcd_id[1] == 0xc4) {
            return N33_LCD_TYPE_38C4;
        }

        if (lcd_id[1] == 0xf7) {
            return N33_LCD_TYPE_38F7;
        }
    }

    if (lcd_id[0] == 0x48 && lcd_id[1] == 0xc4) {
        return N33_LCD_TYPE_48C4;
    }

    return N33_LCD_TYPE_UNSUPPORTED;
}

static const char *n33_lcd_get_name(enum n33_lcd_type type)
{
    if (type == N33_LCD_TYPE_UNSUPPORTED) {
        return n33_lcd_type_unsupported;
    }

    if (type < N33_LCD_TYPE_UNSUPPORTED || type >= N33_LCD_TYPE_COUNT) {
        return n33_lcd_type_unknown;
    }

    return n33_lcd_type_names[type];
}

void n33_lcd_init(void) {
    enum n33_lcd_type type = n33_lcd_get_type();
    const char *name = n33_lcd_get_name(type);
    printf("LCD:   Type %s (%d)\n", name, type);
}
