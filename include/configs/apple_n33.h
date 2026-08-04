/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2026 The freemyipod team (freemyipod.org)
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CFG_SYS_UBOOT_BASE		CONFIG_TEXT_BASE
#define CFG_SYS_INIT_RAM_ADDR	0x22000000
// Just guessing.
#define CFG_SYS_INIT_RAM_SIZE	0x00030000

#define CFG_EXTRA_ENV_SETTINGS "dfu_alt_info=kernel ram 0x0A000000 0x01000000\0"

#endif	/* __CONFIG_H */
