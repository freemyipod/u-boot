#include <init.h>
#include <asm/io.h>
#include <asm/arch-s5l87xx/s5l87xx.h>
#include <linux/delay.h>

struct s5l8730_lcdcon {
    uint32_t con;    // 0x00
    uint32_t cmd;    // 0x04
    uint32_t unk1;   // 0x08
    uint32_t unk2;   // 0x0C
    uint32_t ack;    // 0x10
    uint32_t read;   // 0x14
    uint32_t unk3;   // 0x18
    uint32_t status; // 0x1C
    uint32_t unk[8]; // 0x20
    uint32_t write;  // 0x40
};

#define S5L8730_LCDCON ((volatile struct s5l8730_lcdcon *)S5L87XX_LCD_BASE)

static void s5l8730_lcdcon_read_byte(uint8_t *out) {
    udelay(100);
    writel(0, &S5L8730_LCDCON->ack);

    uint32_t status;
    do {
        status = readl(&S5L8730_LCDCON->status);
    } while((status & 1) == 0);

    udelay(100);

    uint32_t data = readl(&S5L8730_LCDCON->read);
    if (out != NULL) {
        *out = (data >> 1);
    }
}

static void s5l8730_lcdcon_wait_ready(void) {
    debug("%s: start...\n", __func__);
    uint32_t status;
    do {
        status = readl(&S5L8730_LCDCON->status);
    } while((status & (1<<4)) != 0);
    debug("%s: done.\n", __func__);
}

void s5l8730_lcdcon_transact_read(uint32_t cmd, uint32_t len, uint8_t *out) {
    writel(0x1000c20, &S5L8730_LCDCON->con);
    s5l8730_lcdcon_wait_ready();
    writel(cmd, &S5L8730_LCDCON->cmd);

    // Discard first byte???
    s5l8730_lcdcon_read_byte(out);

    for (uint32_t i = 0; i < len; i++) {
        s5l8730_lcdcon_read_byte(out);
        debug("%s: out: %02x\n", __func__, *out);
        out++;
    }
}
