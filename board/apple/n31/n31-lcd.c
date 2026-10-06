#include <asm/io.h>
#include <asm/arch-s5l87xx/s5l87xx.h>
#include <asm/arch-s5l87xx/s5l87xx-clk.h>
#include <linux/iopoll.h>
#include <command.h>
#include <vsprintf.h>

#define S5L8740_LCD_BASE        0x38300000

#define S5L8740_LCD_CON			0x00 /* Control register. */
#define S5L8740_LCD_WCMD		0x04 /* Write command register. */
#define S5L8740_LCD_RCMD		0x0C /* Read command register. */
#define S5L8740_LCD_RDATA		0x10 /* Read data register. */
#define S5L8740_LCD_DBUFF		0x14 /* Read Data buffer */
#define S5L8740_LCD_INTCON		0x18 /* Interrupt control register */
#define S5L8740_LCD_STATUS		0x1C /* LCD Interface status 0106 */
#define S5L8740_LCD_PHTIME		0x20 /* Phase time register 0060 */
#define S5L8740_LCD_RST_TIME	0x24 /* Reset active period 07FF */
#define S5L8740_LCD_DRV_RST		0x28 /* Reset drive signal */
#define S5L8740_LCD_WDATA		0x40 /* Write data register (0x40...0x5C) FIXME */

#define S5L8740_LCD_STATUS_BUSY	0x10

#define S5L8740_LCD_TIMEOUT_US 1

#define WIDTH 240
#define HEIGHT 432

#define readl_poll_timeout_atomic readl_poll_timeout

static int draw(struct cmd_tbl *cmdtp, int flag, int argc,
               char *const argv[])
{
    int result;
    u32 val;

    u32 *src = (u32 *)hextoul(argv[1], NULL);

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            result = readl_poll_timeout(
                S5L8740_LCD_BASE + S5L8740_LCD_STATUS,
                val,
                !(val & S5L8740_LCD_STATUS_BUSY),
                50000
            );

            if (result) {
                printf("timeout at x=%u y=%u\n", x, y);
                return result;
            }

            int bmp_y = HEIGHT - (y + 1);
            writel(src[bmp_y * WIDTH + x], S5L8740_LCD_BASE + S5L8740_LCD_WDATA);
        }
    }

    return 0;
}

U_BOOT_CMD(draw, 2, 1, draw, "draw", "");
