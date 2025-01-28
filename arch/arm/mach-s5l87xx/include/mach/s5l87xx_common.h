/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef S5L87XX_COMMON_H
#define S5L87XX_COMMON_H

#include <linux/types.h>

void s5l87xx_reset_cpu(void);
void s5l87xx_enable_clkgate(const char *id);

#endif //S5L87XX_COMMON_H