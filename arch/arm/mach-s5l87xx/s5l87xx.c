#include <init.h>
#include <asm/io.h>
#include <asm/arch-s5l87xx/s5l87xx.h>

DECLARE_GLOBAL_DATA_PTR;

void reset_cpu(void) {
    svc32_mode_en();
    // According to S5L8700X datasheet
    // rSWRCON = 0xA5 triggers a Software Reset
    // rWDTCON = 0x100000 is not documented but might trigger a Watchdog Reset
    // writel(0xA5, S5L87XX_SWRCON);

#if IS_ENABLED(CONFIG_S5L8701)
    writel(0x110AFF, S5L87XX_WDTCON);
    writel(0xff0, S5L87XX_WDTCNT);
    writel(0x1100FF, S5L87XX_WDTCON);
#else
    writel(0x100000, S5L87XX_WDTCON);
#endif

    while (1)
        ;	/* loop forever till reset */
}
