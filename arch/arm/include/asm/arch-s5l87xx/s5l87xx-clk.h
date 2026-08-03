/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __S5L87XX_CLK_H_
#define __S5L87XX_CLK_H_

#include <inttypes.h>

/*
 * Ungate a single clock gate by raw {gate, bit}. Unlike s5l87xx_enable_clkgate()
 * this needs no device tree, so it is safe in pre-relocation paths (e.g. the
 * debug UART init, which runs before the FDT is set up).
 */
void s5l87xx_enable_clkgate_bit(uint8_t gate, uint8_t bit);
void s5l87xx_enable_clkgate(const char *id);

#endif //__S5L87XX_CLK_H_
