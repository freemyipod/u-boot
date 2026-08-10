/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2026 The freemyipod team (freemyipod.org)
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CFG_SYS_UBOOT_BASE		CONFIG_TEXT_BASE

// 128K or more SRAM, the rest is untested
#define CFG_SYS_INIT_RAM_ADDR	0x22000000
#define CFG_SYS_INIT_RAM_SIZE	0x00020000

#endif	/* __CONFIG_H */
