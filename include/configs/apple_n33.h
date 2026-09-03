/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2026 The freemyipod team (freemyipod.org)
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CFG_SYS_UBOOT_BASE		CONFIG_TEXT_BASE

// 448K SRAM total, first 64K and the 256K at 0x30000 are the fastest, 128K at 0x10000 are slower than DRAM
#define CFG_SYS_INIT_RAM_ADDR	0x22000000
#define CFG_SYS_INIT_RAM_SIZE	0x00070000

#endif	/* __CONFIG_H */
