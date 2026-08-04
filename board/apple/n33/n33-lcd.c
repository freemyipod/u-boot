/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2026 The freemyipod team (freemyipod.org)
 */

#include <init.h>
#include <asm/arch/lcdcon.h>

enum n33_lcd_type {
    N33_LCD_TYPE_UNSUPPORTED = -1,
    N33_LCD_TYPE_38B3 = 0,
    N33_LCD_TYPE_38F7,
    N33_LCD_TYPE_48C4,
};

static enum n33_lcd_type n33_lcdcon_get_type(void) {
    uint8_t id[3] = {0};
    s5l8730_lcdcon_transact_read(4, 3, id);
    if (id[0] == 0x38) {
        if (id[1] == 0xb3) {
            return N33_LCD_TYPE_38B3;
        }
        if (id[1] == 0xf7) {
            return N33_LCD_TYPE_38F7;
        }
    }
    if (id[0] == 0x48 && id[1] == 0xc4) {
        return N33_LCD_TYPE_48C4;
    }
    return N33_LCD_TYPE_UNSUPPORTED;
}

void n33_lcd_init(void) {
    enum n33_lcd_type type = n33_lcdcon_get_type();
    const char* types = "unknown";
    switch (type) {
        case N33_LCD_TYPE_48C4:
            types = "48c4";
            break;
        case N33_LCD_TYPE_38B3:
            types = "38b3";
            break;
        case N33_LCD_TYPE_38F7:
            types = "38f7";
            break;
        case N33_LCD_TYPE_UNSUPPORTED:
            types = "unsupported";
            break;
    }
    printf("LCD:   Type %s (%d)\n", types, type);
}
