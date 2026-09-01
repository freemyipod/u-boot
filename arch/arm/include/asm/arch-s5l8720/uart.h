/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Heungjun Kim <riverful.kim@samsung.com>
 */

#ifndef __ASM_ARCH_UART_H_
#define __ASM_ARCH_UART_H_

#ifndef __ASSEMBLY__

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
};

#endif	/* __ASSEMBLY__ */

#endif
