/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __S5L87XX_H_
#define __S5L87XX_H_

#include <linux/types.h>

#if IS_ENABLED(CONFIG_S5L8701)
#define S5L87XX_LCD_BASE 0x38600000
#else
#define S5L87XX_LCD_BASE 0x38300000
#endif

#if IS_ENABLED(CONFIG_S5L8701)
#define S5L87XX_OTG_BASE 0x38800000
#else
#define S5L87XX_OTG_BASE 0x38400000
#endif

#define S5L87XX_VIC_BASE         0x38E00000
#define S5L87XX_VIC_OFFSET       0x1000
#define S5L87XX_VICBASE(v)       (S5L87XX_VIC_BASE + S5L87XX_VIC_OFFSET * (v))
#define S5L87XX_VICINTENCLEAR(v) (S5L87XX_VICBASE(v) + 0x14)

#define S5L87XX_INT_BASE 0x39C00000
/*
 * Determines which interrupt source is masked.
 * The masked interrupt source will not be serviced.
 */
#define S5L87XX_INTMSK   (S5L87XX_INT_BASE + 0x08)

#define IRQ_USB_FUNC 16

#define S5L87XX_PHY_BASE 0x3C400000

#define S5L87XX_CLK_BASE  0x3C500000

#if IS_ENABLED(CONFIG_S5L8701)
#define S5L87XX_SWRCON    (S5L87XX_CLK_BASE + 0x30) /* Software reset control register */
#define S5L87XX_PWRCON(i) (S5L87XX_CLK_BASE + ((i) == 1 ? 0x40 : 0x28))
#else
#define S5L87XX_SWRCON    (S5L87XX_CLK_BASE + 0x50)
#define S5L87XX_PWRCON(i) (S5L87XX_CLK_BASE \
                                + ((i) == 4 ? 0x6C : \
                                  ((i) == 3 ? 0x68 : \
                                  ((i) == 2 ? 0x58 : \
                                  ((i) == 1 ? 0x4C : \
                                              0x48)))))
#endif

#define S5L87XX_TIMER_BASE 0x3C700000

#define S5L87XX_WDT_BASE 0x3C800000
#define S5L87XX_WDTCON   S5L87XX_WDT_BASE			/* Control Register */
#define S5L87XX_WDTCNT   (S5L87XX_WDT_BASE + 0x04) 	/* 11-bits internal counter */

#define S5L87XX_GPIO_BASE    0x3CF00000
#define S5L87XX_PCON(n) (S5L87XX_GPIO_BASE + 0x00 + (n) * 0x20)
#define S5L87XX_PUNB(n) (S5L87XX_GPIO_BASE + 0x0c + (n) * 0x20)
#define S5L87XX_PUNC(n) (S5L87XX_GPIO_BASE + 0x10 + (n) * 0x20)
#define S5L87XX_PPIE(n) (S5L87XX_GPIO_BASE + 0x14 + (n) * 0x20)

#define S5L87XX_PCON_PINS_CLEAR_FN(rx_pin, tx_pin) (~(GENMASK(((rx_pin + 1) * 4) - 1, rx_pin * 4) |\
GENMASK(((tx_pin + 1) * 4) - 1, tx_pin * 4)))
#define S5L87XX_PCON_PINS_SET_FN(rx_pin, tx_pin, fn) ((fn << (rx_pin * 4)) | (fn << (tx_pin * 4)))

#define S5L87XX_BUS_BASE 0x3E000000

/* Move 0xd3 value to CPSR register to enable SVC mode */
#define svc32_mode_en() __asm__ __volatile__				\
			("@ I&F disable, Mode: 0x13 - SVC\n\t"		\
			 "msr     cpsr_c, %0\n\t" : : "r"(0x13|0xC0))

#endif //__S5L87XX_H_
