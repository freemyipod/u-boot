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
};

static enum n46_lcd_type n46_lcd_get_type(void)
{
    int retry = 3;
    uint8_t lcd_id[4];

    while (retry--) {
        s5l8702_lcdcon_write_config(0x80000c20);
        s5l8702_lcdcon_recv_cmd8(4, 4, lcd_id);

        if (lcd_id[1] == 0x58)
            return N46_LCD_TYPE_58XX;
        else if (lcd_id[1] == 0x38) {
            if      (lcd_id[2] == 0xb3) return N46_LCD_TYPE_38B3;
            else if (lcd_id[2] == 0xc4) return N46_LCD_TYPE_38C4;
            else if (lcd_id[2] == 0xd5) return N46_LCD_TYPE_38D5;
            else if (lcd_id[2] == 0xe6) return N46_LCD_TYPE_38E6;
        }
    }

    return N46_LCD_TYPE_UNSUPPORTED;
}

void n46_lcd_init(void)
{
    s5l87xx_enable_clkgate("lcd");

    s5l8702_lcdcon_write_config(0x81100db8);
    s5l8702_lcdcon_write_phtime(0x33);

    enum n46_lcd_type type = n46_lcd_get_type();
    const char *types = "unknown";
    switch (type) {
        case N46_LCD_TYPE_38B3: types = "38b3"; break;
        case N46_LCD_TYPE_38C4: types = "38c4"; break;
        case N46_LCD_TYPE_38D5: types = "38d5"; break;
        case N46_LCD_TYPE_38E6: types = "38e6"; break;
        case N46_LCD_TYPE_58XX: types = "58xx"; break;
        case N46_LCD_TYPE_UNSUPPORTED: types = "unsupported"; break;
    }
    printf("LCD:   Type %s (%d)\n", types, type);
}
