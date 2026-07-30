/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __S5L8702_LCDCON_H_
#define __S5L8702_LCDCON_H_

void s5l8702_lcdcon_recv_cmd8(uint8_t cmd, int len, uint8_t *buf);
void s5l8702_lcdcon_write_config(uint32_t config);
void s5l8702_lcdcon_write_phtime(uint32_t phtime);

#endif //__S5L8702_LCDCON_H_
