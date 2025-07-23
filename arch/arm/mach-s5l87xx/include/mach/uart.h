/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Heungjun Kim <riverful.kim@samsung.com>
 */

#ifndef __ASM_ARCH_UART_H_
#define __ASM_ARCH_UART_H_

#ifndef __ASSEMBLY__
/* baudrate rest value */
union br_rest {
	unsigned short	slot;		/* udivslot */
	unsigned char	value;		/* ufracval */
};

struct s5p_uart {
	unsigned int	ulcon;
	unsigned int	ucon;
	unsigned int	ufcon;
	unsigned int	umcon;
	unsigned int	utrstat;
	unsigned int	uerstat;
	unsigned int	ufstat;
	unsigned int	umstat;
	unsigned int	utxh;
	unsigned int	urxh;
	unsigned int	ubrdiv;
	union br_rest	rest;
	unsigned char	res3[0x3fd0];
};

static inline int s5p_uart_divslot(void)
{
#if IS_ENABLED(CONFIG_TARGET_IPODNANO2G) || IS_ENABLED(CONFIG_TARGET_N33)
	return 1;
#elif IS_ENABLED(CONFIG_TARGET_N31)
	return 0;
#endif
}

#endif	/* __ASSEMBLY__ */

#endif
